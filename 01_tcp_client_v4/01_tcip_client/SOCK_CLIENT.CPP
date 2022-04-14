/* Code for example client program that uses TCP: IPv4 */
#ifndef unix
#include <winsock2.h>
#include <ws2tcpip.h>
/* also include Use ws2_32.lib library for linking in linker input options */
#define inet_pton InetPtonA
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#define SOCKET int
#define closesocket close
/* also include xnet library for linking  on command line add: -lxnet */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT "1200"                 /* default protocol port number */
#define HOST "localhost"            /* default destination address  */

/*------------------------------------------------------------------------
* Program:   TCP client using IPv4
*
* Purpose:   allocate a socket, connect to a server, and print all output
*
*------------------------------------------------------------------------
*/
int main()
{
    struct  addrinfo addr_req;   /* default address parameters (hints) */
    struct  addrinfo* addr_res;  /* ptr to the  address for connection */
    SOCKET  sd;                  /* socket descriptor - integer        */

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
    addr_req.ai_socktype = SOCK_STREAM;
    addr_req.ai_family = AF_INET; /* Use: AF_INET6 or AF_INET or AF_UNSPEC */
    if (0 != getaddrinfo(HOST, PORT, &addr_req, &addr_res)) {
        fprintf(stderr, "cannot set up the destination address\n");
        exit(1);
    }

    /* Create a socket. */
    sd = socket(addr_res->ai_family, addr_res->ai_socktype, addr_res->ai_protocol);
    if (sd < 0) {
        fprintf(stderr, "socket creation failed\n");
        freeaddrinfo(addr_res);
        exit(1);
    }

    /* Connect the socket to the specified server. */
    if (0 != connect(sd, addr_res->ai_addr, addr_res->ai_addrlen)) {
        fprintf(stderr, "connection failed\n");
        freeaddrinfo(addr_res);
        exit(1);
    }
    freeaddrinfo(addr_res);

    /* Note:    May want/need to read, verify and dispose of
    the initial greeting sent to us by the server       */

    {
        char buf[1000];     /* buffer for data sent to the server */
        snprintf(buf, sizeof(buf), "Automated Client\r\n");
        send(sd, buf, strlen(buf), 0);
    }

    {
        int  n;             /* number of characters read           */
        char buf[1000];     /* buffer for data from the server     */
        /* Repeatedly read data from socket and write to user's screen. */
        n = recv(sd, buf, sizeof(buf), 0);
        while (n > 0) {
            fwrite(buf, n, 1, stdout);
            n = recv(sd, buf, sizeof(buf), 0);
        }
    }

    /* Close the socket. */
    closesocket(sd);

#ifdef WIN32
    WSACleanup();                    /* release use of winsock.dll */
#endif

    /* Terminate the client program gracefully. */
    return(0);
}
