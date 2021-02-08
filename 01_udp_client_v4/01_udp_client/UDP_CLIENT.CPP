/* * code for example client program that uses UDP * */
#ifndef unix
#include <winsock2.h>
#include <ws2tcpip.h>
/* also include Ws2_32.lib library in linking options */
#else
#define closesocket close
#define SOCKET int
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h> // for timeout option
/* also include xnet library for linking; on command line add: -lxnet */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT "1200"                 /* default protocol port number */
#define HOST "localhost"            /* default destination address  */

/*------------------------------------------------------------------------
 * Program:   UDP client using IPv4
 *
 * Purpose:   allocate a socket, connect to a server, and print all output
 *
 *------------------------------------------------------------------------
 */
int main()
{
    struct  addrinfo addr_req;   /* default address parameters (hints) */
    struct  addrinfo *addr_res;  /* ptr to the  address for connection */
    SOCKET  sd;                         /* socket descriptor - integer */

#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(0x0101, &wsaData) != 0)
    {
        fprintf(stderr, "Windows Socket Init failed: %d\n", GetLastError());
        exit(1);
    }
#endif

    /* Convert host name and port name and address hints to the address */
    memset(&addr_req, 0, sizeof(addr_req));
    addr_req.ai_socktype = SOCK_DGRAM;
    addr_req.ai_family = AF_INET; // Use: AF_INET6 or AF_INET or AF_UNSPEC
    if (0 != getaddrinfo(HOST, PORT, &addr_req, &addr_res)) {
        fprintf(stderr, "cannot set up the destination address\n");
        exit(1);
    }

    /* Create a socket. */
    sd = socket(addr_res->ai_family, addr_res->ai_socktype, addr_res->ai_protocol);
    if (sd < 0) {
        fprintf(stderr, "socket creation failed\n");
        exit(1);
    }

    {
        /* Set timeout option for the socket */
#ifndef unix
        int timeout = 100; // in milliseconds
#else
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000L;
#endif
        if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
            fprintf(stderr, "Warning! setting timeout failed\n");
        }
    }

    {
        int  n;              /* number of characters received       */
        int  m;              /* number of characters sent back      */
        char buf[1000];      /* buffer for data from the server     */

        /* Send data to socket in order to request reply. */
        m = 1;              /* sned zero bytes */

        m = sendto(sd,buf,m,0,addr_res->ai_addr,addr_res->ai_addrlen);
        if(m<0)
        {
            fprintf(stderr,"Error in sending");
        }
        else
        {
            struct sockaddr_storage src_addr;
            socklen_t src_addr_len = sizeof(src_addr);

            /* Read data from socket and write to user's screen. */
            n = recvfrom(sd,buf,sizeof(buf),0,(struct sockaddr*)&src_addr,&src_addr_len);
            if (n >= 0) {
                buf[n]='\0';    /* just in case place the termination at the end of the string */
                printf("%s", buf);
            } else {
                fprintf(stderr, "Receiving timeout");
            }
        }
    }

    /* Close the socket. */
    closesocket(sd);

#ifdef WIN32
    WSACleanup();                      /* release use of winsock.dll */
#endif

    /* Terminate the client program gracefully. */
    return(0);
}
