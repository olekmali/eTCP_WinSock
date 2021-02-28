/* * TNPLIB.C ECE-eTCP Course Library * */
#include "tnplib.h"

#include <cstring>
#include <cstdlib>

#ifdef _DEBUG
#include <iostream>
#endif

#ifndef unix
#define socklen_t int
#else
#define closesocket close
#define ioctlsocket ioctl
#endif

int  SocketLibStart() {
#ifdef WIN32
    WSADATA wsaData;
    return( WSAStartup(0x0101, &wsaData) );      /* register with winsock.dll */
#else
    return(0);
#endif
}

void SocketLibEnd() {
#ifdef WIN32
    WSACleanup(); /* release of winsock.dll */
#endif
}

SOCKET TCPStartServer(const int port, const int queue, const int reuse) {
    struct  sockaddr_in sad; /* structure to hold server's address  */
    SOCKET  sd;              /* socket descriptors                  */

    memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET;           /* set family to Internet   */
    sad.sin_addr.s_addr = htonl(INADDR_ANY); /* set the local IP address */
    sad.sin_port = htons((u_short)port);   /* use given port number */

    /* Create a socket */
    sd = socket(PF_INET, SOCK_STREAM, 0);
    if (int(sd)<0) {
#ifdef _DEBUG
        std::cerr << "socket creation failed" << std::endl;
#endif
        return(-1);
    }

    if (reuse!=0 && setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,sizeof(reuse)) <0 ) {
#ifdef _DEBUG
        std::cerr << "reuse address option failed" << std::endl;
#endif
        return(-1);
    }

    /* Bind a local address to the socket */
    if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
#ifdef _DEBUG
        std::cerr << "bind failed" << std::endl;
#endif
        return(-1);
    }

    /* Specify size of request queue */
    if (listen(sd, queue) < 0) {
#ifdef _DEBUG
        std::cerr << "listen failed" << std::endl;
#endif
        return(-1);
    }

    return(sd);
}

SOCKET TCPWaitForConnection(SOCKET sd) {
    struct  sockaddr_in cad; /* structure to hold ClientTCP's address  */
    socklen_t alen;          /* length of address                   */
    alen = sizeof(cad);
    return( accept(sd, (struct sockaddr *)&cad, &alen) );
    /* Note: we could get the ClientTCP's location from cad structure */
}

int TCPStopServerTCP(SOCKET sd) {
    return( closesocket(sd) );
}

SOCKET TCPStartClient(const char *host, const char *port) {
    struct  addrinfo addr_req;   /* default address parameters (hints) */
    struct  addrinfo *addr_res;  /* ptr to the  address for connection */
    int     sd;                  /* socket descriptor                  */

                                 /* Convert host name and port name and address hints to the address */
    memset(&addr_req, 0, sizeof(addr_req));
    addr_req.ai_socktype = SOCK_STREAM;
    addr_req.ai_family = AF_INET; // Use: AF_INET6 or AF_INET or AF_UNSPEC
    if (0 != getaddrinfo(host, port, &addr_req, &addr_res)) {
#ifdef _DEBUG
        std::cerr << "cannot set up the destination address" << std::endl;
#endif
        return(-1);
    }

    /* Create a socket. */
    sd = (int)socket(addr_res->ai_family, addr_res->ai_socktype, addr_res->ai_protocol);
    if (sd < 0) {
#ifdef _DEBUG
        std::cerr << "socket creation failed" << std::endl;
#endif
        freeaddrinfo(addr_res);
        return(-1);
    }

    /* Connect the socket to the specified server. */
    if (connect(sd, addr_res->ai_addr, addr_res->ai_addrlen) < 0) {
#ifdef _DEBUG
        std::cerr << "connection failed" << std::endl;
#endif
        freeaddrinfo(addr_res);
        return(-1);
    }
    freeaddrinfo(addr_res);

    return(sd);
}

int TCPStopClient(SOCKET sd) {
    return( closesocket(sd) );
}

int TCPSetTTL(SOCKET sd, int ttl) {
    return( setsockopt(sd,IPPROTO_IP,IP_TTL,(const char*)&ttl,sizeof(ttl)) );
}

int TCPSetTimeout(SOCKET sd, int tout) {
    return( setsockopt(sd,SOL_SOCKET,SO_RCVTIMEO,(char*)&tout,sizeof(tout)) );
}

int TCPSetFragment(SOCKET sd, int isOk) {
    return(0);
}

int TCPSetSocketReuse(SOCKET sd, int isOk) {
    return(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&isOk, sizeof(isOk)));
}

int TCPSetNoDelay(SOCKET sd, int isOk) {
#ifdef TCP_NODELAY
    return( setsockopt(sd,IPPROTO_TCP,TCP_NODELAY,(char*)&isOk,sizeof(isOk)) );
#else
#ifdef _DEBUG
    std::cerr << "TCP_NODELAY not supported by this platform - option ignored" << std::endl;
#endif
#endif
    return(0);
}

int TCPPrepClose(SOCKET sd) {
    return( shutdown(sd,SD_BOTH) );
}

int TCPRecvAny(SOCKET sd, char *buffer, const int maxsize) {
    return( recv(sd,buffer,maxsize,0) );
}

int TCPRecvLine(SOCKET sd, char *line, const int maxsize) {
    int  status;
    int  len=0;
    char bch;
    while (1) {
        status = recv(sd,&bch,1,0);
        if (status<0) return(status);
        if (len>=maxsize) return(-1); /* someone overruns buffer, disconnect! */
        if (bch=='\n')
        {
            line[len]='\0';
            return(len);
        }
        else if (bch=='\r')
        {
            /* ignore \r */
        }
        else
        {
            line[len]=bch;
            len++;
        }
    }
}

int TCPRecvDumpLine(SOCKET sd) {
    int  status;
    int  len=0;
    char bch;
    while (1) {
        status = recv(sd,&bch,1,0);
        if (status<0) return(status);
        if (bch=='\n') return(len);
        if (bch!='\r') len++;
    }
}

int TCPSendAny(SOCKET sd, const char *buffer, int size) {
    return( send(sd,buffer,size,0) );
}

int TCPSendLine(SOCKET sd, const char *line) {
    return( send(sd,line,int(strlen(line)),0) );
}

SOCKET UDPStartServer(const int port) {
    return( UDPStartMServer(port, 0) );
}

SOCKET UDPStartMServer(const int port, int multiple) {
    struct  sockaddr_in sad; /* structure to hold server's address  */
    SOCKET  sd;              /* socket descriptors                  */

    /* Create a socket */
    sd = socket(PF_INET, SOCK_DGRAM, 0);
    if (int(sd)<0) {
#ifdef _DEBUG
        std::cerr << "socket creation failed" << std::endl;
#endif
        return(-1);
    }

    if (multiple!=0 && setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,(char*) &multiple,sizeof(multiple)) <0 ) {
#ifdef _DEBUG
        std::cerr << "reuse address option failed" << std::endl;
#endif
        return(-1);
    }

    memset((char*)&sad,0,sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET;           /* set family to Internet   */
    sad.sin_addr.s_addr = htonl(INADDR_ANY);   /* set the local IP address */
    sad.sin_port = htons((u_short)port);   /* use given port number */

    /* Bind a local address to the socket */
    if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
#ifdef _DEBUG
        std::cerr << "bind failed" << std::endl;
#endif
        return(-1);
    }

    return(sd);
}

int UDPStopServer(SOCKET sd) {
    return( closesocket(sd) );
}

SOCKET UDPStartClient() {
    SOCKET sd;
    sd=socket(PF_INET,SOCK_DGRAM,0);
    if (int(sd)<0) {
#ifdef _DEBUG
        std::cerr << "socket creation failed" << std::endl;
#endif
        return(-1);
    }
    return(sd);
}

int UDPStopClient(SOCKET sd) {
    return( closesocket(sd) );
}

int UDPSetTTL(SOCKET sd, int ttl) {
    return( setsockopt(sd,IPPROTO_IP,IP_TTL,(const char*)&ttl,sizeof(ttl)) );
}

int UDPSetTimeout(SOCKET sd, int tout) {
    return( setsockopt(sd,SOL_SOCKET,SO_RCVTIMEO,(char*)&tout,sizeof(tout)) );
}

int UDPSetFragment(SOCKET sd, int isOk) {
    return(0);
}

int UDPSetSocketReuse(SOCKET sd, int isOk) {
    return( setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,(char*) &isOk,sizeof(isOk)) );
}

int UDPPrepClose(SOCKET sd) {
    return( shutdown(sd,SD_BOTH) );
}

int UDPMulticastSetTTL(SOCKET sd, int ttl) {
    return( setsockopt(sd,IPPROTO_IP,IP_MULTICAST_TTL,(char*) &ttl,sizeof(ttl)) );
}

int UDPMulticastJoin(SOCKET sd, const char* address) {
    struct ip_mreq mreq;
    inet_pton(AF_INET, address, &mreq.imr_multiaddr.s_addr);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    /* check given address is multicast */
    if(!IN_MULTICAST(ntohl(mreq.imr_multiaddr.s_addr))) {
#ifdef _DEBUG
        std::cerr << "invalid multicast address: " << address << std::endl;
#endif
        return(-1);
    }
    return( setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) );
}

int UDPMulticastDrop(SOCKET sd, const char* address) {
    struct ip_mreq mreq;
    inet_pton(AF_INET, address, &mreq.imr_multiaddr.s_addr);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    /* check given address is multicast */
    if(!IN_MULTICAST(ntohl(mreq.imr_multiaddr.s_addr))) {
#ifdef _DEBUG
        std::cerr << "invalid multicast address: " << address << std::endl;
#endif
        return(-1);
    }
    return( setsockopt(sd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) );
}

int UDPRecvAny(SOCKET sd, char *buffer, const int maxsize, sockaddr_in* remoteaddr) {
    socklen_t alen = sizeof(*remoteaddr);
    return( recvfrom(sd,buffer,maxsize,0,(sockaddr *)remoteaddr,&alen) );
}

int UDPSendAny(SOCKET sd, const char *buffer, int size, const sockaddr_in* remoteaddr) {
    socklen_t alen = sizeof(*remoteaddr);
    return( sendto(sd,buffer,size,0,(sockaddr *)remoteaddr,alen) );
}

void CreateAddress(const char* address, const char *port, sockaddr_in * sad) {
    struct  addrinfo addr_req;   /* default address parameters (hints) */
    memset(&addr_req, 0, sizeof(addr_req));
    addr_req.ai_family = AF_INET;
    addr_req.ai_socktype = SOCK_DGRAM;
    struct  addrinfo* addr_res;  /* ptr to the resolved address */
    getaddrinfo(address, port, &addr_req, &addr_res);
    memcpy(sad, addr_res->ai_addr, addr_res->ai_addrlen );
}
