#include "variables.h"

unsigned long deviceID   = 0;
double       posX       = 0.0;
double       posY       = 0.0;
int          needtostop = 0;

SOCKET      mcastsocket=0;
sockaddr_in mcastaddr;

const int   MPORT = 60001;
const char* MADDR = "224.2.2.2";

void NetComStart() {
    SocketLibStart();
    mcastsocket = UDPStartMServer(MPORT, 1);
    UDPMulticastJoin(mcastsocket , MADDR);
    UDPMulticastSetTTL(mcastsocket, 16);
    mcastaddr=CreateAddress(MADDR, MPORT);
    UDPSetTimeout(mcastsocket, 100);
}

void NetComStop() {
    UDPMulticastDrop(mcastsocket , MADDR);
    UDPStopServer(mcastsocket);
    mcastsocket=0;
    SocketLibEnd();
 }

#include <cstdlib>
#include <ctime>

void InitRandom() {
    std::time_t TimeInSeconds;
    std::time( &TimeInSeconds );
    std::srand((unsigned int)TimeInSeconds);
}


void InitDeviceID() {
    std::time_t TimeInSeconds;
    std::time( &TimeInSeconds );
    deviceID = static_cast<unsigned long>(TimeInSeconds);
}


#include <iostream>

void D(const char *errormessage) {
#ifdef _DEBUG
    std::cerr << errormessage << std::endl;
#endif
}


