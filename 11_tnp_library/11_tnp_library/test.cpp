//* Sending e-mail with SMTP *


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


int SendEmailSteps(SOCKET s, const char* message) {
    const int bufsize=256;
    char buffer[bufsize];

    TCPRecvLine(s, buffer, bufsize);
    D(buffer);
    if (buffer[0]!='2') return(-2);

    TCPSendLine(s, "HELO MyComputerName.bradley.edu\r\n");
    TCPRecvLine(s, buffer, bufsize);
    D(buffer);
    if (buffer[0]!='2') return(-3);

    TCPSendLine(s, "MAIL FROM: olekmali@bradley.edu\r\n");
    TCPRecvLine(s, buffer, bufsize);
    D(buffer);
    if (buffer[0]!='2') return(-4);
 
    TCPSendLine(s, "RCPT TO: olekmali@bradley.edu\r\n");
    TCPRecvLine(s, buffer, bufsize);
    D(buffer);
    if (buffer[0]!='2') return(-5);

    TCPSendLine(s, "DATA\r\n");
    TCPRecvLine(s, buffer, bufsize);
    D(buffer);
    if (buffer[0]!='3') return(-6);

    TCPSendLine(s, message);
    TCPSendLine(s, "\n.\r\n");
    TCPRecvLine(s, buffer, bufsize);
    D(buffer);
    if (buffer[0]!='2') return(-7);
    
    return(0);
}


int SendEmail(const char* message) {
    SOCKET s;

    s=TCPStartClient("gdansk.bradley.edu", "25");
    if (!isValidSocket(s)) {
        D("Cannot open connection to the SMTP server");
        return(-1);
    }

    int status=SendEmailSteps(s, message);

    TCPSendLine(s, "QUIT\r\n");
    TCPRecvDumpLine(s);

    TCPPrepClose(s);
    TCPStopClient(s);
    return(status);
}




int main()
{
    SocketLibStart();

    int st = SendEmail("This laptop is still alive.\nWill report again soon.\r\n");
    if ( st<0) {
        cerr << "Problem: " << st<< endl;
    } else {
        cerr << "Message sent" << endl;
    }

    SocketLibEnd();
    return(0);
}

