//* A simple UDP Multicast service discovery with updated list of services *
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#include "tnplib.h"

#ifdef unix
#include <unistd.h> /* needed for sleep(sec) */
#else
#include <windows.h> /* needed for Sleep(ms) */
inline void sleep (int sec) { Sleep(sec*1000); } 
#endif

#include <cstring>
#include <iostream>
using namespace std;

const char* MPORT = "60001";
const char* MADDR = "224.2.2.2";
const char* QSERA = "?any";     // any device type query
const char* QSERD = "?sensor";  // this device type query
const char* RSERD = "!sensor";  // this device type reply


inline void D(const char *errormessage) {
#ifdef _DEBUG
    cerr << errormessage << endl;
#endif
}

void MUDPSniff(const char * addr, const char * port) {
    SOCKET ms;
    sockaddr_in recvAddr, sendAddr;

    char buffer[256];

    ms = UDPStartMServer(atoi(port), 1);
    UDPMulticastJoin(ms , addr);
    UDPMulticastSetTTL(ms, 16);
    CreateAddress(addr, port, &sendAddr);

    UDPSetTimeout(ms, 1000);
    for (;;) {
        strcpy(buffer, QSERA);
        UDPSendAny(ms, buffer, strlen(buffer)+1, &sendAddr);
        D(buffer);

        for (;;) {
            int status;
            status=UDPRecvAny(ms, buffer, sizeof(buffer), &recvAddr);
            if (status<0) break;
            buffer[255]='\0'; // just in case terminate the string
            if (buffer[0]=='!') {
                cout << buffer << " found at " << inet_ntoa(recvAddr.sin_addr) << endl;
            }
        }
        sleep(1);
    }

    UDPMulticastDrop(ms , addr);
    UDPStopServer(ms);
}



int main()
{
    SocketLibStart();

    MUDPSniff(MADDR, MPORT);

    SocketLibEnd();
    return(0);
}
