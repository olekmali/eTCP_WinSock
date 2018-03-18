//* A simple Web Server with a password-protected page *
#include "tnplib.h"

#include <iostream>
#include <cstdio>
#include <cstring>
using namespace std;

inline void D(const char *errormessage) {
#ifdef _DEBUG
    cerr << errormessage << endl;
#endif
}

void CreateReply(const char * status, const char * title, const char * message, const char * realm, char * buffer) {
    strcpy(buffer, "HTTP/1.1 ");
    strcat(buffer, status);
    strcat(buffer, "\r\n");
    strcat(buffer, "Server: eTCP Demo Server\r\n");
    // Date: must be provided unless the server has no reliable clock - then it must not be provided
    strcat(buffer, "Connection: close\r\n");
    strcat(buffer, "Cache-Control: no-cache\r\nExpires: 0\r\nETag: \"\"\r\n");
    strcat(buffer, "Content-Type: text/html; charset=UTF-8\r\n");
    if (realm!=0) {
        strcat(buffer, "WWW-Authenticate: Basic realm=\""); strcat(buffer, realm); strcat(buffer, "\"\r\n");
    }
    strcat(buffer, "\r\n");

    // HTML Page header
    strcat(buffer, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\r\n");
    strcat(buffer, "<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">\r\n");
    strcat(buffer, "<html>\r\n<head><title>");
    strcat(buffer, title);
    strcat(buffer, "</title></head>\r\n<body>\r\n");

    strcat(buffer, message);

    // HTML Page footer
    strcat(buffer, "\r\n</body>\r\n</html>\r\n");
}

void CreateStatusReply(const char * status, char * buffer) {
    char tit[128];
    char msg[128];

    strcpy(tit, "Error ");
    strcat(tit, status);

    strcpy(msg, "<h1>Error ---</h1><p>");
    msg[10]=status[0];
    msg[11]=status[1];
    msg[12]=status[2];
    strcat(msg, status);
    strcat(msg, "</p>");
    CreateReply(status, tit, msg, 0, buffer);
}

void CreateErrorReply(int error, char * buffer) {
    switch(error) {
        case 400: CreateStatusReply( "400 Bad Request",           buffer); break;
        case 401: CreateReply(       "401 Unauthorized",
                                     "401 Unauthorized",
                                     "401 Password required for Vault Owner",
                                              "Vault Owner Only", buffer);
                                                                           break;
        case 402: CreateStatusReply( "402 Payment Required",      buffer); break;
        case 403: CreateStatusReply( "403 Forbidden",             buffer); break;
        case 404: CreateStatusReply( "404 Not Found",             buffer); break;
        case 405: CreateStatusReply( "405 Method Not Allowed",    buffer); break;
        case 406: CreateStatusReply( "406 Not Acceptable",        buffer); break;
        case 407: CreateStatusReply( "407 Proxy Authentication Required", buffer); break;
        case 411: CreateStatusReply( "411 Length Required",       buffer); break;
        case 414: CreateStatusReply( "414 Request-URI Too Large", buffer); break;
        case 500: CreateStatusReply( "500 Internal Server Error", buffer); break;
        case 501: CreateStatusReply( "501 Not Implemented",       buffer); break;
        case 503: CreateStatusReply( "503 Service Unavailable",   buffer); break;
        default:  CreateReply(       "500 Internal Server Error", "500 Internal Server Error",
                                     "500 Internal Server Error: Out of range error message", 0, buffer);
                                                                           break;
    }
}

void CreateContentReply(const char * title, const char * message, char * buffer) {
    CreateReply("200 Ok", title, message, 0, buffer);
}

void CreateRedirection(const char * whereto, char * buffer) {
    strcpy(buffer, "HTTP/1.1 301 Moved Permanently\r\n");
    strcat(buffer, "Location: ");
    strcat(buffer, whereto);
    strcat(buffer, "\r\n");
    strcat(buffer, "Server: eTCP Demo Server\r\n");
    strcat(buffer, "Connection: close\r\n");
    strcat(buffer, "\r\n");
}


void ParseURL(const char * buffer, char * method, char * localurl, char * params) {
    const char * s;
    int    j;

    method[0]='\0';
    localurl[0]='\0';
    params[0]='\0';

    s=buffer;

    j=0;
    while (*s!=' ' && j<15) {
        method[j]=*s;
        ++s;
        ++j;
    }
    method[j]='\0';

    if (*s!=' ') {
        method[0]='\0';
        return;
    }

    ++s;
    if (strncmp( s, "http://", 7)==0) {
        s=s+7;
        while (*s!='\0' && *s!='/') ++s;
        if (*s!='/') return;
    }

    j=0;
    while (*s!='\0' && *s!=' ' && *s!='?') {
        localurl[j]=*s;
        ++s;
        ++j;
    }
    localurl[j]='\0';

    if (*s=='?') {
        ++s;
        j=0;
        while (*s!='\0' && *s!=' ') {
            params[j]=*s;
            ++s;
            ++j;
        }
        params[j]='\0';
    }
}

const char * page12 = "<h1>Device Status</h1><p>The device is on.</p>";
const char * page13 = "<h1>Device Status</h1><p>The device is off.</p>";
const char * page22 = "<h1>Device On</h1><p>The device was already on.</p>";
const char * page23 = "<h1>Device On</h1><p>The device is now on.</p>";
const char * page32 = "<h1>Device Off</h1><p>The device is now off.</p>";
const char * page33 = "<h1>Device Off</h1><p>The device was already off.</p>";
const char * menuln = "<hr /><p> <a href=\"/\">status</a> &nbsp; <a href=\"/on.html\">switch on</a> &nbsp; <a href=\"/off.html\">switch off</a> </p>";

      // ***** user-ID "Aladdin" and password "open sesame" results in *****
      char   passwordline[80]="Authorization: Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ=="; 
      // global static var, can be changed, redefined to be stored in EPROM, saved to v-disk, etc.
      // Needed: base64 encoder code for userid:password string to be able to update the passwordline

void SimpleHTTPServer(int port) {
    char buffer[2048];  // make sure that the Web page contents fit inside
    char webpage[4096]; // make sure that the HTTP reply and the Web page fit inside
    char method[16];    // GET POST HEAD PUT, 16 is already too much space
    char localurl[128];
    char params[256];
    int  status;
    int  passok=0;

    int  mode =0;

    SOCKET serve = TCPStartServer(port, 2, 1);

    for (;;) {
        SOCKET visitor = TCPWaitForConnection(serve);

        status = TCPRecvLine(visitor, buffer, sizeof(buffer) );
        if (status<0) continue;
        D(buffer);
        ParseURL(buffer, method, localurl, params);

        do {
            status = TCPRecvLine(visitor, buffer, sizeof(buffer) );
            if (strcmp(buffer, passwordline)==0) passok=1;
        } while (status>0);

        if (passok==0) {
            CreateErrorReply(401, webpage);
        } else if (strcmp(method, "GET")!=0) {
            CreateErrorReply(400, webpage);
        } else if (strcmp(localurl, "/")==0) {
            if (mode==0) {
                strcpy(buffer, page13);
            } else {
                strcpy(buffer, page12);
            }
            strcat(buffer, menuln);
            CreateContentReply("eTCP Web Server", buffer, webpage);
        } else if (strcmp(localurl, "/on.html")==0) {
            if (mode==0) {
                strcpy(buffer, page23);
                mode=1;
                cout << "Switching on" << endl << flush;
            } else {
                strcpy(buffer, page22);
            }
            strcat(buffer, menuln);
            CreateContentReply("eTCP Web Server", buffer, webpage);
        } else if (strcmp(localurl, "/off.html")==0) {
            if (mode==0) {
                strcpy(buffer, page33);
            } else {
                strcpy(buffer, page32);
                mode=0;
                cout << "Switching off" << endl << flush;
            }
            strcat(buffer, menuln);
            CreateContentReply("eTCP Web Server", buffer, webpage);
        } else {
            CreateErrorReply(404, webpage);
        }

        TCPSendLine(visitor, webpage);
        D(webpage);

        TCPPrepClose(visitor);
        TCPStopClient(visitor);
        D("Done");
    }
}



int main()
{
    SocketLibStart();

    SimpleHTTPServer(8088);

    SocketLibEnd();
    return(0);
}
