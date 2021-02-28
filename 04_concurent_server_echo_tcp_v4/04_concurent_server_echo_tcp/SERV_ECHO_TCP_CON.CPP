/* * Concurent Echo Servers on port 60000 for Transfer Control Protocol * */
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

#define PROTOPORT 60000
                             /* default protocol port number        */
#define QLEN 5
                             /* size of request queue               */

int OneTCPEcho(SOCKET s);

int main(int argc, char *argv[])
{
    struct  sockaddr_in sad; /* structure to hold server's address  */
    struct  sockaddr_in cad; /* structure to hold client's address  */
    SOCKET  sd;              /* server socket descriptor            */
    int     port;            /* protocol port number                */
    int     alen;            /* length of address                   */
    fd_set  rfds;            /* read file descriptor set            */
    fd_set  afds;            /* active file descriptor set          */

#ifdef WIN32
    WSADATA wsaData;
    if(WSAStartup(0x0101, &wsaData)!=0)
    {
        fprintf(stderr, "Windows Socket Init failed: %d\n", GetLastError());
        exit(1);
    }
#endif

    memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
    
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
    sad.sin_family = AF_INET;         /* set family to Internet     */
    sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address   */
    
    /* Create a socket */
    sd = socket(PF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        fprintf(stderr, "socket creation failed\n");
        exit(1);
    }
    
    /* Bind a local address to the socket */
    if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        fprintf(stderr,"bind failed\n");
        exit(1);
    }

    /* Specify size of request queue */
    if (listen(sd, QLEN) < 0) {
        fprintf(stderr,"listen failed\n");
        exit(1);
    }

    FD_ZERO(&afds);
    FD_SET(sd, &afds);

    while (1) {
        unsigned int fdndx;
        int status;
        memcpy(&rfds, &afds, sizeof(rfds));

        status=select(FD_SETSIZE, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0 );
        if (status<0) {
            fprintf(stderr,"select failed\n");
            exit(1);
        }

        if (FD_ISSET(sd, &rfds)) {
            SOCKET    sd2;

            alen = sizeof(cad);
            sd2 = accept(sd, (struct sockaddr *)&cad, &alen);
            if (sd2<0) {
                fprintf(stderr,"accept failed\n");
                exit(1);
            }
            FD_SET(sd2, &afds);
        }

#ifndef unix
        for (fdndx=0; fdndx<rfds.fd_count; ++fdndx) {
            SOCKET sd3=rfds.fd_array[fdndx];
#else
        for (SOCKET sd3=0; sd3<FD_SETSIZE; ++sd3) {
#endif

            if (sd3!=sd && FD_ISSET(sd3, &rfds))
                if (OneTCPEcho(sd3)==0) {
                    (void) closesocket(sd3);
                    FD_CLR(sd3, &afds);
                }
        }
    }
    closesocket(sd);

#ifdef WIN32
    WSACleanup();
#endif

    return(0);
}

int OneTCPEcho(SOCKET s)
{
    char    buf[65536];      /* buffer for string the echoing       */
    int     n;               /* number of characters received       */
    int     m;               /* number of characters sent back      */

    n = recv(s, buf, sizeof(buf), 0);
    
    if (n < 0)
    {
        fprintf(stderr,"Error in receiving\n");
    }
    else if (n==0) {
        /* Remote closed the connection */
    } 
    else if (n > 0)
    {
        m = send(s,buf,n,0);
        if(m<0){
            fprintf(stderr,"Error in sending\n");
        }
    }

    return(n);
}
