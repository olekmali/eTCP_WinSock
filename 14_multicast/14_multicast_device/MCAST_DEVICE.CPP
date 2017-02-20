//* A simple UDP Multicast device reporter *
#include "tnplib.h"

#include <iostream>
#include <cstring>
using namespace std;

const int   MPORT = 60001;
const char* MADDR = "224.2.2.2";
const char* QSERA = "?any";     // any device type query
const char* QSERD = "?sensor";  // this device type query
const char* RSERD = "!sensor";  // this device type reply


inline void D(const char *errormessage) {
#ifdef _DEBUG
    cerr << errormessage << endl;
#endif
}

void MUDPDevice(const char * addr, int port) {
    SOCKET ms;
    sockaddr_in recvAddr, sendAddr;

    char buffer[256];

    ms = UDPStartMServer(port, 1);
    UDPMulticastJoin(ms , addr);
    UDPMulticastSetTTL(ms, 16);
    sendAddr=CreateAddress(addr, port);

    /* Initial send without being asked */
    strcpy(buffer, RSERD);
    strcat(buffer, " ");
    UDPSendAny(ms, buffer, strlen(buffer)+1, &sendAddr);

    for (;;) {
        UDPRecvAny(ms, buffer, sizeof(buffer), &recvAddr);
        buffer[255]='\0'; // just in case terminate the string
        if ( (strcmp(buffer, QSERA)==0) || (strcmp(buffer, QSERD)==0) ) {
            strcpy(buffer, RSERD);
            strcat(buffer, " ");
            UDPSendAny(ms, buffer, strlen(buffer)+1, &sendAddr);
            D(buffer);
        }
    }

    UDPMulticastDrop(ms , addr);
    UDPStopServer(ms);
}



int main()
{
    SocketLibStart();

    MUDPDevice(MADDR, MPORT);

    SocketLibEnd();
    return(0);
}
