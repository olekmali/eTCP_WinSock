/* * tnplib.h ECE-eTCP Course Library * */
#ifndef _TNPLIB
#define _TNPLIB

#ifndef unix

#include <winsock2.h>
#include <ws2tcpip.h>

inline int isValidSocket(SOCKET sd) { return(sd!=INVALID_SOCKET); }
//#define isValidSocket(sd) (sd!=INVALID_SOCKET)
/* also include Use ws2_32.lib library for linking in linker input options */

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#define SOCKET int

inline int isValidSocket(SOCKET sd) { return(sd>=0); }
//#define isValidSocket(sd) (sd>=0)
/* also include xnet library for linking  on command line add: -lxnet */

#endif

#ifndef SD_BOTH
#define SD_BOTH 2
#endif


int     SocketLibStart();
void    SocketLibEnd();

SOCKET  TCPStartServer(const int port, const int queue, const int reuse);
SOCKET  TCPWaitForConnection(SOCKET sd);
int     TCPStopServerTCP(SOCKET sd);
SOCKET  TCPStartClient(const char *dest, const char *port);
int     TCPStopClient(SOCKET sd);

int     TCPSetTTL(SOCKET sd, int ttl);
int     TCPSetTimeout(SOCKET sd, int tout);
int     TCPSetFragment(SOCKET sd, int isOk);
int     TCPSetSocketReuse(SOCKET sd, int isOk);
int     TCPSetNoDelay(SOCKET sd, int isOk);
int     TCPPrepClose(SOCKET sd);

// returns number of bytes read, 0 if remote closed, <0 if error
int     TCPRecvAny(SOCKET sd, char *buffer, const int maxsize);
int     TCPRecvLine(SOCKET sd, char *line, const int maxsize);
int     TCPRecvDumpLine(SOCKET sd);
// returns number of bytes sent, <0 if error
int     TCPSendAny(SOCKET sd, const char *buffer, int size);
int     TCPSendLine(SOCKET sd, const char *line);


SOCKET  UDPStartServer(const int port);
SOCKET  UDPStartMServer(const int port, int multiple); // allows multiple bind to port
int     UDPStopServer(SOCKET sd);
SOCKET  UDPStartClient();
int     UDPStopClient(SOCKET sd);

int     UDPSetTTL(SOCKET sd, int ttl);
int     UDPSetTimeout(SOCKET sd, int tout);
int     UDPSetFragment(SOCKET sd, int isOk);
int     UDPSetSocketReuse(SOCKET sd, int isOk);
int     UDPPrepClose(SOCKET sd);

int     UDPMulticastSetTTL(SOCKET sd, int ttl);
int     UDPMulticastJoin(SOCKET sd, const char* address);
int     UDPMulticastDrop(SOCKET sd, const char* address);

int     UDPRecvAny(SOCKET sd, char *buffer, const int maxsize, sockaddr_in* remoteaddr);
// returns number of bytes read, 0 if remote closed, <0 if error
int     UDPSendAny(SOCKET sd, const char *buffer, int size, const sockaddr_in* remoteaddr);
// returns number of bytes sent, <0 if error

sockaddr_in CreateAddress(const char* address, const char *port);


#endif
