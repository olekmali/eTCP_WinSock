/* * Simple Echo Servers on port 60000 for Transfer Control Protocol * */
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

#define PROTOPORT       60000
                             /* default protocol port number        */
#define QLEN 5
                                   /* size of request queue        */

int main(int argc, char *argv[])
{
    struct  sockaddr_in sad; /* structure to hold server's address  */
    struct  sockaddr_in cad; /* structure to hold client's address  */
    SOCKET  sd, sd2;         /* socket descriptors                  */
    int     port;            /* protocol port number                */

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
    
    /* In case the server crashes allow to reuse the port number ASAP without socket timeout */
    {
        const int on = 1;
        if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) < 0) {
            fprintf(stderr, "Warining: in case of failure the port number cannot be reused immediately\n");
        }

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
    
    /* Main server loop - accept and handle requests */
    while (1)
    {
        int     alen;            /* length of address                   */
        char    buf[1000];       /* buffer for string the echoing       */
        int     n;               /* number of characters received       */
        int     m;               /* number of characters sent back      */
        alen = sizeof(cad);
        sd2=accept(sd, (struct sockaddr *)&cad, &alen);
        if ( sd2 < 0) {
            fprintf(stderr, "accept failed\n");
            exit(1);
        }
        
        while (1)
        {
            n = recv(sd2, buf, sizeof(buf), 0);
            
            if (n<0)
            {
                fprintf(stderr,"Error in receiving\n");
                break;
            }
            else if (n==0) {
                /* Remote closed the connection */
                break;
            }
            else if(n>0)
            {
                m = send(sd2,buf,n,0);
                if(m<0){
                    fprintf(stderr,"Error in sending\n");
                    break;
            }
        }
    }
    closesocket(sd2);
    }

#ifdef WIN32
    WSACleanup();
#endif

    return(0);
}
