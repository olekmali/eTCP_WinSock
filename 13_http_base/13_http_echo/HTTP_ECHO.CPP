//* A HTTP Echo Web Server that allows to inspect what is being sent to server *
#include "tnplib.h"

#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <cstdio>
#include <cstring>
using namespace std;

inline void D(const char *errormessage) {
#ifdef _DEBUG
    cerr << errormessage << endl;
#endif
}

void SimpleHTTPServer(int port) {
    char buffer[256];
    int status;
    SOCKET serve = TCPStartServer(port, 2, 1);

    for (;;) {
        SOCKET visitor = TCPWaitForConnection(serve);

        status = TCPRecvLine(visitor, buffer, sizeof(buffer) );
        if (status<0) continue;
        D(buffer);

        // HTTP Header
        TCPSendLine(visitor, "HTTP/1.1 200 Ok\r\n");
        TCPSendLine(visitor, "Server: eTCP HTTP Echo Server\r\n");
        // Date: must be provided unless the server has no reliable clock - then it must not be provided
        TCPSendLine(visitor, "Connection: close\r\n");
        TCPSendLine(visitor, "Cache-Control: no-cache\r\nExpires: 0\r\nETag: \"\"\r\n");
        TCPSendLine(visitor, "Content-Type: text/html; charset=UTF-8\r\n");
        TCPSendLine(visitor, "\r\n");

        // HTML Page header
        TCPSendLine(visitor, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\r\n");
        TCPSendLine(visitor, "<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">\r\n");
        TCPSendLine(visitor, "<html>\r\n<head><title>A simple dynamic Web page</title></head>\r\n<body>\r\n");

        TCPSendLine(visitor, "<pre>"); 
        TCPSendLine(visitor, buffer); 
        TCPSendLine(visitor, "\r\n"); 

        for (;;) {
            status = TCPRecvLine(visitor, buffer, sizeof(buffer) );
            if (status<0) break;
            if (strlen(buffer)==0) break;
            TCPSendLine(visitor, buffer); 
            TCPSendLine(visitor, "\r\n"); 
        }

        TCPSendLine(visitor, "</pre>\r\n");

        // HTML Page footer
        TCPSendLine(visitor, "</body>\r\n</html>\r\n");
        
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
