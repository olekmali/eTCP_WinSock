/* * Multicasting listener (server) * */
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

#define DEFAULT_IP "224.2.2.2"
#define DEFAULT_PORT 60001
#define MAX_MSG 100

int main(int argc, char *argv[]) {

    SOCKET          sd;
    int             rc, n;
    struct sockaddr_in cliAddr, servAddr;
    int             cliLen;
    struct in_addr  mcastAddr;
    struct ip_mreq  mreq;
    struct hostent *h;
    unsigned short  p;
    int             ttl;
    int             one;
    char            msg[MAX_MSG+1];

#ifdef WIN32
    WSADATA wsaData;
    if(WSAStartup(0x0101, &wsaData)!=0)
    {
        fprintf(stderr, "Windows Socket Init failed: %d\n", GetLastError());
        exit(1);
    }
#endif

    /* get mcast address to listen to */
    if(argc>3) {
        printf("usage : %s [mcast address=%s] [port number=%d]\n", argv[0], DEFAULT_IP, DEFAULT_PORT);
        exit(0);
    } else if (argc>2) {
        h=gethostbyname(argv[1]);
        p =(unsigned short)atol(argv[2]);
    } else if (argc>=2) {
        h=gethostbyname(argv[1]);
    } else {
        h=gethostbyname(DEFAULT_IP);
        p=DEFAULT_PORT;
    }
    if(h==NULL) {
        printf("%s : unknown group %s\n", argv[0], argv[1]);
        exit(1);
    }
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

    /* allow multiple bind port */
    one = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one));

    /* bind port */
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servAddr.sin_port=htons(p);  
    if(bind(sd,(struct sockaddr *) &servAddr, sizeof(servAddr))<0) {
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

    printf("%s : listening to mgroup %s:%d\n", argv[0], inet_ntoa(mcastAddr), p);
    

    /* infinite server loop */
    while(1) {
        cliLen=sizeof(cliAddr);
        n = recvfrom(sd, msg, MAX_MSG, 0, (struct sockaddr *)&cliAddr, &cliLen);
        if(n<0) {
            printf("%s : cannot receive data\n", argv[0]);
            continue;
        }
        msg[n]='\0';
    
        printf("%s : received from %s:%d: %s\n", argv[0], inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port), msg);
    }/* end of infinite server loop */

    /* leave the group after you are done - actually never happens in this example */
    rc = setsockopt(sd,IPPROTO_IP,IP_DROP_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
    if(rc<0) {
        printf("%s : cannot leave multicast group %s\n", argv[0], inet_ntoa(mcastAddr));
        exit(1);
    }

#ifdef WIN32
    WSACleanup();                      /* release use of winsock.dll */
#endif

    return 0;
}
