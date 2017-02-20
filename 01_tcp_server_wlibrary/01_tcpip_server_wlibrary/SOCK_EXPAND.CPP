/* * expanded server.c - program that uses TCP, IPv4, IPv6 * */
#ifndef unix
#include <winsock2.h>
#include <ws2tcpip.h>
/* also include Ws2_32.lib library for linking 
   MsVC - AltF7 + linking options */
#else
#define closesocket close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
/* also include xnet library for linking 
   on command line add: -lxnet */
#endif

#include <iostream>
#include <cstdio>
#include <cstring>
using namespace std;

/*------------------------------------------------------------------------
 * Program:  server
 *
 * Purpose:  allocate a socket and then repeatedly execute the following:
 *    (1) wait for the next connection from a ClientTCP
 *    (2) send a short message to the ClientTCP
 *    (3) close the connection
 *    (4) go back to step (1)
 *
 *------------------------------------------------------------------------
 */

int  SocketLibStart();
void SocketLibEnd();

int TCPStartServer(const int port, const int queue, const int reuse);
int TCPWaitForConnection(const int sd);
int TCPStopServerTCP(const int sd);
int TCPStartClient(const char *dest, const char *port);
int TCPStopClient(const int sd);

int TCPRecvAny(const int socket, char *buffer, const int maxsize);
// returns number of bytes read, 0 if remote closed, <0 if error
int TCPSendAny(const int socket, const char *buffer, int size);
// returns number of bytes sent, <0 if error
int TCPRecvLine(const int socket, char *line, const int maxsize);
int TCPSendLine(const int socket, const char *line);



int main()
{
    int sd1, sd2;           /* server and ClientTCP socket descriptors */

    SocketLibStart();

    sd1=TCPStartServer(60000, 6, 1);
    if ( sd1<0) {
        cerr << "Server cannot be started - aborting" << endl;
        exit(1);
    }

    {
        char buf[81];        /* buffer for string the server sends  */
        char nam[81];        /* buffer for string the server recvs  */
        int  visits = 0;     /* counts ClientTCP connections           */

        /* Main server loop - accept and handle requests */
        while (1) {
            sd2=TCPWaitForConnection(sd1);
            if ( sd2<0) {
                cerr << "accept failed" << endl;
                exit(1);
            }
            visits++;

            TCPSendLine(sd2,"Hello, what is your name?");
            TCPRecvLine(sd2, nam, 80);

            snprintf(buf,sizeof(buf),"It is nice to meet you %s, you are connection number %d today.\n",
                nam, visits);
            TCPSendLine(sd2,buf);
            // TCPSendLine(sd2,"Good Bye!\n");
            closesocket(sd2);
        }
    }

    SocketLibEnd();

    return(0);
}


int  SocketLibStart() {
#ifdef WIN32
    WSADATA wsaData;
    return WSAStartup(0x0101, &wsaData);      /* register with winsock.dll */
#else
    return 0;
#endif
}

void SocketLibEnd() {
#ifdef WIN32
    WSACleanup(); /* release of winsock.dll */
#endif
}


int TCPStartServer(const int port, const int queue, const int reuse) {
    struct  protoent *ptrp;  /* pointer to a protocol table entry   */
    struct  sockaddr_in sad; /* structure to hold server's address  */
    int     sd;              /* socket descriptors                  */

    memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET;           /* set family to Internet   */
    sad.sin_addr.s_addr = INADDR_ANY;   /* set the local IP address */

    sad.sin_port = htons((u_short)port);   /* use given port number */

    /* Map TCP transport protocol name to protocol number */
    ptrp = getprotobyname("tcp");
    if ( ptrp == 0) {
        cerr << "cannot map \"tcp\" to protocol number" << endl;
        return(-1);
    }

    /* Create a socket */
    sd = (int)socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (socket < 0) {
        cerr << "socket creation failed" << endl;
        return(-1);
    }

    if (reuse!=0 && setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,sizeof(reuse)) <0 ) {
        cerr << "reuse address option failed" << endl;
        return(-1);
    }

    /* Bind a local address to the socket */
    if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        cerr << "bind failed" << endl;
        return(-1);
    }

    /* Specify size of request queue */
    if (listen(sd, queue) < 0) {
        cerr << "listen failed" << endl;
        return(-1);
    }

    return(sd);
}

int TCPWaitForConnection(const int sd) {
    struct  sockaddr_in cad; /* structure to hold ClientTCP's address  */
    int     alen;            /* length of address                   */
    alen = sizeof(cad);
    return( (int)accept(sd, (struct sockaddr *)&cad, &alen) );
    /* Note: we could get the ClientTCP's location from cad structure */
}

int TCPStopServerTCP(const int sd) {
    return closesocket(sd);
}

int TCPStartClient(const char *host, const char *port) {
    struct  addrinfo addr_req;   /* default address parameters (hints) */
    struct  addrinfo *addr_res;  /* ptr to the  address for connection */
    int     sd;              /* socket descriptor                   */

    /* Convert host name and port name and address hints to the address */
    memset(&addr_req, 0, sizeof(addr_req));
    addr_req.ai_socktype = SOCK_STREAM;
    addr_req.ai_family = AF_INET; // Use: AF_INET6 or AF_INET or AF_UNSPEC
    if (0 != getaddrinfo(host, port, &addr_req, &addr_res)) {
        fprintf(stderr, "cannot set up the destination address\n");
        return(-1);
    }

    /* Create a socket. */
    sd = (int)socket(addr_res->ai_family, addr_res->ai_socktype, addr_res->ai_protocol);
    if (sd < 0) {
        cerr << "socket creation failed" << endl;
        freeaddrinfo(addr_res);
        return(-1);
    }

    /* Connect the socket to the specified server. */
    if (connect(sd, addr_res->ai_addr, addr_res->ai_addrlen) < 0) {
        cerr << "connection failed" << endl;
        freeaddrinfo(addr_res);
        return(-1);
    }
    freeaddrinfo(addr_res);

    return(sd);
}

int TCPStopClient(const int sd) {
    return closesocket(sd);
}


int TCPRecvAny(const int socket, char *buffer, const int maxsize) {
    return( recv(socket,buffer,maxsize,0) );
}

int TCPSendAny(const int socket, const char *buffer, int size) {
    return( send(socket,buffer,size,0) );
}

int TCPRecvLine(const int socket, char *line, const int maxsize) {
    int     n;
    int     len=0;
    char    bch;
    n = recv(socket,&bch,1,0);
    while (n > 0) {
        if (len>=maxsize) return(-1); /* someone overruns buffer, disconnect! */
        if (bch!='\n'&&bch!='\r')
        {
            line[len]=bch;
            len++;
        }
        else
        {
            line[len]='\0';
            if (len>0) return(len);
            /* else - an empty string, skip it */
        }
        n = recv(socket,&bch,1,0);
    }
    return(n);
}

int TCPSendLine(const int socket, const char *line) {
    return( (int)send(socket,line,strlen(line),0) );
}
