/* * Multicasting sender (client) * */
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
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
/* also include xnet library for linking; on command line add: -lxnet */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_IP      "224.2.2.2"
#define DEFAULT_PORT    "60001"

int main(int argc, char *argv[]) {
    struct  protoent* ptrp;     /* pointer to a protocol table entry   */
    SOCKET  sd;                         /* socket descriptor - integer */
    int rc;                                      /* return status code */

#ifdef WIN32
    WSADATA wsaData;
    if(WSAStartup(0x0101, &wsaData)!=0)
    {
        fprintf(stderr, "Windows Socket Init failed: %u\n", GetLastError());
        exit(1);
    }
#endif

    if (argc<4) {
        printf("usage %s <maddress> <port> <data1> <data2> ... <dataN>\n", argv[0]);
        exit(1);
    }
    
    char* host = argv[1];
    char* port = argv[2];
    unsigned int port_nr = atoi(port);

    /* check given address is multicast */
//  TO DO: if (!IN_MULTICAST( ????? )) { }

    if (port_nr < 1024 || port_nr > 65535) {
        fprintf(stderr, "invalid port number range %u\n", port_nr);
        exit(1);
    }

    ptrp = getprotobyname("udp");
    if (ptrp == 0) {
        fprintf(stderr, "cannot map \"udp\" to protocol number\n");
        exit(1);
    }

    /* Create a socket. */
    sd = socket(PF_INET, SOCK_DGRAM, ptrp->p_proto);
    if (sd < 0) {
        fprintf(stderr, "socket creation failed\n");
        exit(1);
    }

    {
        // enable reuse of the same port (useful for all servers and multicast receviers)
        unsigned int option = 1;
        rc = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));
                            //  ^^^ group of socket options (at socket level, at IP lever, at TCP or UDP lelvel
                            //              ^^^ actual socket option (number)
                            //                            ^^^ void pointer to option data (e.g. int or a complex structure)
                            //                                             ^^ how large is the data variable in bytes
        if ( rc<0 ) {
            fprintf(stderr, "option SO_REUSEADDR failed\n");
            // closesocket(sd); exit(1);
        }
    }

    {
        // set multicast TTL - range
        unsigned char ttl = 1;
        rc = setsockopt(sd, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl));
        if (rc < 0) {
            fprintf(stderr, "Cannot set multicast TTL value\n");
            // closesocket(sd); exit(1);
        }
    }


    /* construct a multicast address structure */
    struct  sockaddr_in mad;    /* structure to hold mulitcast address */
    memset(&mad, 0, sizeof(mad));
    mad.sin_family = AF_INET;
    mad.sin_addr.s_addr = inet_addr(host);
    mad.sin_port = htons(port_nr);

    /* send data */
    for(int i=3; i<argc; i++) {
        const char* msg = argv[i];
        const unsigned int len = strlen(msg);
        rc = sendto(sd, msg, len, 0, (struct sockaddr*)&mad, sizeof(mad));
        
        if (rc<0) {
            fprintf(stderr, "cannot send message number %d\n", i-2);
            break;
        } else {
            printf("message nr %d of %u bytes sent\n",i-2, len );
        }
        
    }/* end for */
    
    /* close socket and exit */
    closesocket(sd);

#ifdef WIN32
    WSACleanup();                      /* release use of winsock.dll */
#endif

    exit(0);
}
