#include <process.h>
/* needed for _beginthread(...), also change compliation to multithreaded code generation */

#include "variables.h"
#include "sensors.h"
#include "database.h"
#include "controler.h"


int main()
{
    InitRandom();
    InitDeviceID();

    while(1) {
        NetComStart();
        if (mcastsocket==0) {
            D("ERROR: Cannot connect to network, will try again in 1000ms.");
            Sleep(1000L);
            continue;
        }

        _beginthread((void (*)(void *))runSensors       , 0, NULL);
        _beginthread((void (*)(void *))runDatabase      , 0, NULL);
        _beginthread((void (*)(void *))runControler     , 0, NULL);

        while (needtostop==0) {
            Sleep( 1000L );     // forever loop with low CPU usage
        }

        Sleep( 10000L );         // wait for threads to shut down gracefully too
        NetComStop();
    }

    return(0);
}
