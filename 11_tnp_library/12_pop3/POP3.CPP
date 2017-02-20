//* Checking e-mail with POP3 *
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


int CheckEmailSteps(SOCKET s) {
    const int bufsize=256;
    char buffer[bufsize];

    TCPRecvLine(s, buffer, bufsize);
    D(buffer);
    if (buffer[0]!='+') return(-2);

    TCPSendLine(s, "USER BUNetID\r\n");
    TCPRecvLine(s, buffer, bufsize);
    D(buffer);
    if (buffer[0]!='+') return(-3);

    TCPSendLine(s, "PASS YourPasswordHere\r\n");
    TCPRecvLine(s, buffer, bufsize);
    D(buffer);
    if (buffer[0]!='+') return(-4);
 
    TCPSendLine(s, "STAT\r\n");
    TCPRecvLine(s, buffer, bufsize);
    D(buffer);
    if (buffer[0]!='+') return(-5);

    char *b = buffer;
    while (*b!=0 && *b!=' ') ++b;
    if (*b==0) return(-6); // a number was expected
    ++b;
    char *c = b;
    while (*c!=0 && *c!=' ') ++c;
    *c=0; // truncate the reminder of the string (destroys the original buffer)
    int num=atoi(b); // extracts next integer 

    return(num);
}


int CheckEmail() {
    SOCKET s;

    s=TCPStartClient("webmail.bradley.edu", "110");
    if (!isValidSocket(s)) {
        D("Cannot open connection to the POP server");
        return(-1);
    }

    int status=CheckEmailSteps(s);

    TCPSendLine(s, "QUIT\r\n");
    TCPRecvDumpLine(s);

    TCPPrepClose(s);
    TCPStopClient(s);
    return(status);
}




int main()
{
    SocketLibStart();

    int st = CheckEmail();
    if ( st<0) {
        cerr << "Problem: " << st << endl;
    } else {
        cerr << "Found: " << st << endl;
    }

    SocketLibEnd();
    return(0);
}
