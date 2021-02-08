/* * server.c - code for example server program that uses UDP * */
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

#define PROTOPORT 1200
                                    /* default protocol port number */
/*------------------------------------------------------------------------
 * Program:  server
 *
 * Purpose:  allocate a socket and then repeatedly execute the following:
 *    (1) wait for the UDP packet from a client
 *    (2) finds out who is a client
 *    (3) send a short reply message to the client
 *    (4) go back to step (1)
 *
 *------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
    struct  protoent *ptrp;  /* pointer to a protocol table entry   */
    struct  sockaddr_in sad; /* structure to hold server's address  */
    SOCKET  sd;              /* socket descriptor - integer         */
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

    memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
    sad.sin_port = htons((u_short)port); /* set server port number  */
    sad.sin_family = AF_INET;           /* set family to Internet   */
    sad.sin_addr.s_addr = INADDR_ANY;   /* set the local IP address */

    /* Map UDP transport protocol name to protocol number */
    ptrp = getprotobyname("udp");
    if ( ptrp == 0) {
        fprintf(stderr, "cannot map \"udp\" to protocol number");
        exit(1);
    }

    /* Create a socket */
    sd = socket(PF_INET, SOCK_DGRAM, ptrp->p_proto);
    if (sd < 0) {
        fprintf(stderr, "socket creation failed");
        exit(1);
    }

    /* Bind a local address to the socket */
    if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        fprintf(stderr, "bind failed");
        exit(1);
    }

    {
        char buf[1000];      /* buffer for string the server sends  */
        int  visits = 0;     /* counts client connections           */

        /* Main server loop - accept and handle requests */
        while (1) {
            struct sockaddr_storage remote_addr;
            socklen_t remote_addr_len = sizeof(remote_addr);
            int  n;          /* number of characters received       */
            int  m;          /* number of characters sent back      */
            n = recvfrom(sd,buf,sizeof(buf),0,(struct sockaddr*)&remote_addr,&remote_addr_len);
            if (n<0)
            {
                fprintf(stderr,"Error in receiving\n");
                continue;
            }
            else if(n>=0) /* We could receive a useful emtpy packet */
            {
                visits++;
                snprintf(buf,sizeof(buf),"This server has been contacted %d time%s\n", 
                                                visits,visits==1?".":"s.");
                m = sendto(sd,buf,strlen(buf)+1,0, (struct sockaddr*)&remote_addr,remote_addr_len);
                if(m<0)
                {
                    fprintf(stderr,"Error in sending");
                    continue;
                }
            }
        }
    }

#ifdef WIN32
    WSACleanup();                       /* release use of winsock.dll */
#endif

    return(0);
}
