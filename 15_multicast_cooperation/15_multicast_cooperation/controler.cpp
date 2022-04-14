#include "controler.h"
#include "variables.h"

#include <cstdlib>

void runControler() {
    while (needtostop==0) {
        Sleep( 10L );
        // do the routine tasks for this thread

        // drive the robot? ;-)
        posX += 0.1*std::rand()/RAND_MAX-0.05;
        posY += 0.1*std::rand()/RAND_MAX-0.05;

    }
}
