//* A simple Web Server with one status page - better design *
#define _CRT_SECURE_NO_WARNINGS 1

#include "tnplib.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;

inline void D(const char *errormessage) {
#ifdef _DEBUG
    cerr << errormessage << endl;
#endif
}

static int counter = 0;

void SimpleHTTPServer(int port) {
    char buffer[256];
    char webpage[4096]; // make sure that the HTTP reply and the Web page fit inside
    int status;
    SOCKET serve = TCPStartServer(port, 2, 1);

    for (;;) {
        SOCKET visitor = TCPWaitForConnection(serve);

        // In this simple server we do not care about the request header
        // but may want to display it for demonstration and debugging purposes
        status = TCPRecvLine(visitor, buffer, sizeof(buffer) );
        if (status<0) continue;
        D(buffer);

        // In this simple server we do not care about the reminder of the request header
        do {
            status = TCPRecvDumpLine(visitor);
        } while (status>0);

        webpage[0]='\0';
        // HTTP Header
        strcat(webpage, "HTTP/1.1 200 Ok\r\n");
        strcat(webpage, "Server: eTCP Simple Demo Server\r\n");
        // Date: must be provided unless the server has no reliable clock - then it must not be provided
        strcat(webpage, "Connection: close\r\n");
        strcat(webpage, "Cache-Control: no-cache\r\nExpires: 0\r\nETag: \"\"\r\n");
        strcat(webpage, "Content-Type: text/html; charset=UTF-8\r\n");
        strcat(webpage, "\r\n");

        // HTML Page header
        strcat(webpage, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\r\n");
        strcat(webpage, "<html>\r\n<head>\r\n");
        strcat(webpage, "<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">\r\n");
        strcat(webpage, "<title>A simple dynamic Web page</title></head>\r\n<body>\r\n");

        strcat(webpage, "<p>"); 
        counter++; _itoa(counter, buffer, 10); strcat(webpage, buffer);
        strcat(webpage, "</p>\r\n");

        // HTML Page footer
        strcat(webpage, "</body>\r\n</html>\r\n");

        TCPSendLine(visitor, webpage);

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
