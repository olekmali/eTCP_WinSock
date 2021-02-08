//* A simple Web Server with one status page - inefficient design *
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

        // HTTP Header
        TCPSendLine(visitor, "HTTP/1.1 200 Ok\r\n");
        TCPSendLine(visitor, "Server: eTCP Simple Demo Server\r\n");
        // Date: must be provided unless the server has no reliable clock - then it must not be provided
        TCPSendLine(visitor, "Connection: close\r\n");
        TCPSendLine(visitor, "Cache-Control: no-cache\r\nExpires: 0\r\nETag: \"\"\r\n");
        TCPSendLine(visitor, "Content-Type: text/html; charset=UTF-8\r\n");
        TCPSendLine(visitor, "\r\n");

        // HTML Page header
        TCPSendLine(visitor, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\r\n");
        TCPSendLine(visitor, "<html>\r\n<head>\r\n");
        TCPSendLine(visitor, "<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">\r\n");
        TCPSendLine(visitor, "<title>A simple dynamic Web page</title>\r\n</head>\r\n<body>\r\n");

        TCPSendLine(visitor, "<p>"); 
        counter++; _itoa(counter, buffer, 10); TCPSendLine(visitor, buffer);
        TCPSendLine(visitor, "</p>\r\n");

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
