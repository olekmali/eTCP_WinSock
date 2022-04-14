/* * Multicasting listener (server) * */
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
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

#define DEFAULT_IP      "224.2.2.2"
#define DEFAULT_PORT    "60001"
#define MAX_MSG_LEN     256

int main(int argc, char *argv[]) {
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

    /* get mcast address to listen to */
    char* host = DEFAULT_IP;
    char* port = DEFAULT_PORT;
    if(argc>3) {
        printf("usage : %s [mcast address=%s] [port number=%d]\n", argv[0], DEFAULT_IP, DEFAULT_PORT);
        exit(0);
    } else if (argc>2) {
        host = argv[1];
        port = argv[2];
    } else if (argc>1) {
        host = argv[1];
    } // else already pre-assigned the defaults
    unsigned int port_nr = atoi(port);

    /* check given address is multicast */
//  TO DO: if (!IN_MULTICAST( ????? )) { }

    if (port_nr < 1024 || port_nr > 65535) {
        fprintf(stderr, "invalid port number range %u\n", port_nr);
        exit(1);
    }

    /* Create a socket. */
    sd = socket(PF_INET, SOCK_DGRAM, 0);
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
        if (rc < 0) {
            fprintf(stderr, "option SO_REUSEADDR failed\n");
            // closesocket(sd); exit(1);
        }
    }

    /* construct a multicast address structure */
    struct sockaddr_in sad;     /* structure to hold mulitcast address */
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = htonl(INADDR_ANY);
    sad.sin_port = htons(port_nr);

    /* bind port */
    if(bind(sd,(struct sockaddr*) &sad, sizeof(sad))<0) {
        fprintf(stderr, "cannot bind UDP port %d\n", port_nr);
        closesocket(sd); exit(1);
    }

    /* construct an IGMP join request structure */
    struct ip_mreq mreq;
    rc = inet_pton(AF_INET, host, &mreq.imr_multiaddr.s_addr); /* multicast group address */
    if (rc <= 0) {
        fprintf(stderr, "invalid address %s\n", host);
        closesocket(sd); exit(1);
    }
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);      /* your network interface(s) */
    rc = setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
    if (rc < 0) {
        fprintf(stderr, "cannot join multicast group %s\n", host);
        closesocket(sd); exit(1);
    }

    printf("listening to mgroup %s:%d\n", host, port_nr);

    /* infinite server loop */
    while(1) {
        char buffer[MAX_MSG_LEN+1]; /* allow an extra char for termianting 0 */
        struct sockaddr_in cad;     /* structure to hold incoming client address */
        int cal =sizeof(cad);
        int n = recvfrom(sd, buffer, MAX_MSG_LEN, 0, (struct sockaddr *)&cad, &cal);
        if(n<0) {
            fprintf(stderr, "error receiving data\n");
            continue;
        }
        buffer[n]='\0';
    
        printf("received from %s:%d:\t%s\n", 
            inet_ntoa(cad.sin_addr), ntohs(cad.sin_port), buffer);

        if (strcmp(buffer, "quit server") == 0) break;
    }/* end of infinite server loop */

    /* leave the group after you are done */
    rc = setsockopt(sd,IPPROTO_IP,IP_DROP_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
    if(rc<0) {
        fprintf(stderr, "cannot leave multicast group %s\n", host);
        closesocket(sd); exit(1);
    }

#ifdef WIN32
    WSACleanup();                      /* release use of winsock.dll */
#endif

    return 0;
}
