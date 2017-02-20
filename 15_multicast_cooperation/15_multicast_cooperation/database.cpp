#include "database.h"
#include "variables.h"

void runDatabase() {
    sockaddr_in senderAddr;
    char buffer[1400];
    while (needtostop==0) {
        Sleep( 10L ); // this time should be much shorter than frequency of incoming messages from all senders
        // do the routine tasks for this thread

        int status;
        status=UDPRecvAny(mcastsocket, buffer, sizeof(buffer), &senderAddr);
        if (status<0) { // pfroblem with socket
            if (WSAGetLastError()!=WSAEWOULDBLOCK) continue; // no data received and time out
            needtostop=1;
            break;
        } else if (status==0) continue; // no data received

        buffer[1399]='\0'; // just in case terminate the string

        if (buffer[0]=='P') {
            // store the new known position for a robot
            // for our robot locate the position using best match and apply correction to the position
            // for other robots locate a robot on the map either by using best match or robot reported position
            // update the map using new sensor information

            D(buffer);
        } // else ...

    }
}

