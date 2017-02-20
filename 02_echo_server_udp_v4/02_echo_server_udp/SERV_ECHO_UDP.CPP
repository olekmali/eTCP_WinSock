/* * Simple Echo Servers on port 60000 for User Datagram Protocol * */
#ifndef unix
#include <winsock2.h>
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

#define PROTOPORT       60000
                             /* default protocol port number        */

int main(int argc, char *argv[])
{
    struct  protoent *ptrp;  /* pointer to a protocol table entry   */
    struct  sockaddr_in sad; /* structure to hold server's address  */
    struct  sockaddr_in cad; /* structure to hold client's address  */
    SOCKET  s, rc;           /* socket descriptors                  */
    int     port;            /* protocol port number                */

#ifdef WIN32
    WSADATA wsaData;
    if(WSAStartup(0x0101, &wsaData)!=0)
    {
        fprintf(stderr, "Windows Socket Init failed: %d\n", GetLastError());
        exit(1);
    }
#endif

    if (argc > 1) {                 /* if argument specified        */
        port = atoi(argv[1]);       /* convert argument to binary   */
    } else {
        port = PROTOPORT;           /* use default port number      */
    }
    
    if (port <= 0)                  /* test for illegal value       */
    {                               /* print error message and exit */
        fprintf(stderr,"bad port number %s\n",argv[1]);
        exit(1);
    }

    sad.sin_port = htons((u_short)port); /* set server port number  */
    sad.sin_family=AF_INET;
    sad.sin_addr.s_addr=INADDR_ANY;
    
    /* Map UDP transport protocol name to protocol number */
    ptrp = getprotobyname("udp");
    if ( ptrp == 0) {
        fprintf(stderr, "cannot map \"udp\" to protocol number");
        exit(1);
    }

    s=socket(AF_INET, SOCK_DGRAM, ptrp->p_proto);
    if(s<0)
    {
        fprintf(stderr,"Socket creation failed\n");
        return 1;
    }
    
    rc=bind(s, (struct sockaddr *)&sad, sizeof(sad));
    if(rc<0)
    {
        fprintf(stderr,"Bind,failed\n");
        return 1;
    }
    
    while(1)
    {
        int     alen;            /* length of address                   */
        char    buf[1000];       /* buffer for string the echoing       */
        int     n;               /* number of characters received       */
        int     m;               /* number of characters sent back      */

        alen = sizeof(cad);
        n = recvfrom(s,buf,sizeof(buf),0,(struct sockaddr*)&cad,&alen);
        if (n<0)
        {
            fprintf(stderr,"Error in receiving\n");
            continue;
        }
        else if (n==0) {
            fprintf(stderr,"Remote sent an empty packet\n");
            continue;
        }
        else if(n>0)
        {
            m = sendto(s,buf,n,0,(struct sockaddr*)&cad,alen);
            if(m<0)
            {
                fprintf(stderr,"Error in sending");
                continue;
            }
        }
    }

#ifdef WIN32
    WSACleanup();
#endif
    return (0);
}
