#ifndef _VARIABLES
#define _VARIABLES

// include BU-ECE Network Library
#include "tnplib.h"

#ifdef unix
#include <unistd.h> /* needed for sleep(sec) */
#else
#include <windows.h> /* needed for Sleep(ms) */
inline void sleep (int sec) { Sleep(sec*1000); } 
#endif


// list below all variables accessible from libraries and declare as extern
// then declare them again inside the variables.cpp and initilaize if possible
extern unsigned long deviceID;
extern double       posX;
extern double       posY;
extern int          needtostop;

extern SOCKET       mcastsocket;
extern sockaddr_in  mcastaddr;


void NetComStart();
void NetComStop();

void InitRandom();
void InitDeviceID();

void D(const char *errormessage);

#endif
