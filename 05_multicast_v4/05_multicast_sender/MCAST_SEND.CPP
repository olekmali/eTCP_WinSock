/* * Multicasting sender (client) * */
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

/* #define DEFAULT_IP "224.2.2.2" */
/* #define DEFAULT_PORT 60001 */

int main(int argc, char *argv[]) {

    SOCKET          sd;
    int             rc, i;
    struct sockaddr_in cliAddr, servAddr;
    struct in_addr  mcastAddr;
    struct ip_mreq  mreq;
    struct hostent *h;
    unsigned short  p;
    unsigned long   addr;
    unsigned char   ttl;
    int             one;

#ifdef WIN32
    WSADATA wsaData;
    if(WSAStartup(0x0101, &wsaData)!=0)
    {
        fprintf(stderr, "Windows Socket Init failed: %d\n", GetLastError());
        exit(1);
    }
#endif

    if(argc<4) {
        printf("usage %s <maddress> <port> <data1> <data2> ... <dataN>\n", argv[0]);
        exit(1);
    }
    
    h = gethostbyname(argv[1]);
    if(h==NULL) {
        printf("%s : unknown host %s\n", argv[0], argv[1]);
        exit(1);
    }
    p =(unsigned short)atol(argv[2]);
    if(p==0) {
        printf("%s : invalid port %s\n", argv[0], argv[2]);
        exit(1);
    }
    
    memcpy((char *) &mcastAddr, h->h_addr_list[0], h->h_length);
  
    /* check given address is multicast */
    if(!IN_MULTICAST(ntohl(mcastAddr.s_addr))) {
        printf("%s : given address '%s' is not multicast\n", argv[0], inet_ntoa(mcastAddr));
        exit(1);
    }

    /* create socket */
    sd = socket(AF_INET,SOCK_DGRAM,0);
    if(sd<0) {
        printf("%s : cannot create socket\n", argv[0]);
        exit(1);
    }


    /* enable multicasting on a network interface
    addr = INADDR_ANY;
    rc = setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&addr, sizeof(addr));
    if (rc<0) {
        printf("%s : cannot set the network interface to enable multicasting\n", argv[0]);
        exit(1);
    } /* */
    
    /* set multicast TTL - range
    ttl = 1;
    rc = setsockopt(sd, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl));
    if (rc<0) {
        printf("%s : cannot set ttl = %d\n", argv[0], ttl);
        exit(1);
    } /* */

    /* allow multiple bind port */
    one = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one));

    /* bind port */
    cliAddr.sin_family=AF_INET;
    cliAddr.sin_addr.s_addr=htonl(INADDR_ANY);
    cliAddr.sin_port=htons(p);  
    if(bind(sd,(struct sockaddr *) &cliAddr, sizeof(cliAddr))<0) {
        printf("%s : cannot bind port %d\n", argv[0], p);
        exit(1);
    }

    /* set multicast TTL - range */
    ttl = 1;
    rc = setsockopt(sd, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl));
    if (rc<0) {
        printf("%s : cannot set ttl = %d\n", argv[0], ttl);
        exit(1);
    }

    /* join multicast group */
    mreq.imr_multiaddr.s_addr=mcastAddr.s_addr;
    mreq.imr_interface.s_addr=htonl(INADDR_ANY);
    rc = setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
    if(rc<0) {
        printf("%s : cannot join multicast group %s\n", argv[0], inet_ntoa(mcastAddr));
        exit(1);
    }


    servAddr.sin_family = h->h_addrtype;
    memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr, h->h_length);
    servAddr.sin_port = htons(p);

    printf("%s : sending data on multicast group %s (%s) port %d\n",argv[0],
                            h->h_name, inet_ntoa(*(struct in_addr *) h->h_addr_list[0]), ntohs(servAddr.sin_port) );

    /* send data */
    for(i=3;i<argc;i++) {
        rc = sendto(sd,argv[i],strlen(argv[i])+1,0, (struct sockaddr *) &servAddr, sizeof(servAddr));
        
        if (rc<0) {
            printf("%s : cannot send data number %d\n", argv[0], i-2);
            break;
        } else {
            printf("%s : sent %d bytes in message %d\n", argv[0], strlen(argv[i])+1, i-2);
        }
        
    }/* end for */
    
    /* close socket and exit */
    closesocket(sd);

#ifdef WIN32
    WSACleanup();                      /* release use of winsock.dll */
#endif

    exit(0);
}
