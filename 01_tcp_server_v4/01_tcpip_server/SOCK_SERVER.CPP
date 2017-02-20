/* Server.c - code for example server program that uses TCP: IPv4 */
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

#define PORT 1200
/* default protocol port number */
#define QLEN 1
/* size of request queue        */
/*------------------------------------------------------------------------
* Program:  server
*
* Purpose:  allocate a socket and then repeatedly execute the following:
*    (1) wait for the next connection from a client
*    (2) send a short message to the client
*    (3) close the connection
*    (4) go back to step (1)
*
*------------------------------------------------------------------------
*/
int main()
{
    struct  protoent *ptrp;     /* pointer to a protocol table entry   */
    struct  sockaddr_in sad;    /* structure to hold server's address  */
    struct  sockaddr_in cad;    /* structure to hold client's address  */
    SOCKET  sd, sd2;            /* socket descriptors - integers       */
    int     alen;               /* length of address                   */

#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(0x0101, &wsaData) != 0)
    {
        fprintf(stderr, "Windows Socket Init failed: %d\n", GetLastError());
        exit(1);
    }
#endif

    memset((char *)&sad, 0, sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET;             /* set family to Internet   */
    sad.sin_addr.s_addr = INADDR_ANY;     /* set the local IP address */

    sad.sin_port = htons((u_short)PORT);  /* use the defined port num */

                                         /* Map TCP transport protocol name to protocol number */
    ptrp = getprotobyname("tcp");
    if (ptrp == 0) {
        fprintf(stderr, "cannot map \"tcp\" to protocol number\n");
        exit(1);
    }

    /* Create a socket */
    sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
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
        fprintf(stderr, "bind failed\n");
        exit(1);
    }

    /* Specify size of request queue */
    if (listen(sd, QLEN) < 0) {
        fprintf(stderr, "listen failed\n");
        exit(1);
    }

    {
        char buf[1000];      /* buffer for string the server sends  */
        char name[80];       /* small buffer for received user data */
        int  visits = 0;     /* counts client connections           */

                             /* Main server loop - accept and handle requests */
        while (1) {
            alen = sizeof(cad);
            sd2 = accept(sd, (struct sockaddr *)&cad, (socklen_t*)&alen);
            if (sd2<0) {
                fprintf(stderr, "accept failed\n");
                exit(1);
            }

            snprintf(buf, sizeof(buf), "Welcome to the server.\r\nWhat is your name please?\r\n");
            send(sd2, buf, strlen(buf), 0);

            { // collect one line of data from the user
                int len = 0;
                char bch;
                while (1) {
                    if (recv(sd2, &bch, 1, 0) < 0) break;
                    else if (bch == '\n') break;
                    else if (bch == '\r') /* ignore \r */;
                    else
                    {
                        name[len] = bch;
                        len++;
                    }
                    if (len + 1 >= sizeof(name)) break; // exit on exhausted data buffer
                }
                name[len] = '\0';
            }

            fprintf(stdout, "%s has just contacted us.\n", name);

            visits++;
            snprintf(buf, sizeof(buf), "Nice to meet you %s.\r\nYou are the %dth  person who stopped by today\r\n", name, visits);
            send(sd2, buf, strlen(buf), 0);
            closesocket(sd2);
        }
    }

#ifdef WIN32
    WSACleanup();                      /* release use of winsock.dll */
#endif

    return(0);
}
