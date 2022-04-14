#include "sensors.h"
#include "variables.h"
#include <stdio.h>

void runSensors() {
    char buf[1400];
    while (needtostop==0) {
        Sleep( 1000L );
        // do the routine tasks for this thread

        snprintf(buf,sizeof(buf), "P %d %.3f %.3f", deviceID, posX, posY);
        UDPSendAny(mcastsocket, buf, int(strlen(buf)+1), &mcastaddr);
    }
}

