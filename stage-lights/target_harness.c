
#ifdef STAGE_LIGHTS_USE_TARGET_HARNESS

#include <time.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


#include "stage_lights.h"


int main(int argc, char * argv[])
{
    struct timespec lastts;
    
    (void)(argc);
   	(void)(argv);
    
    clock_gettime(CLOCK_MONOTONIC, &lastts);
    
    slInitialize(argc, argv);

	while(true)
    {
        struct timespec ts;
        uint64_t time_ns;

        clock_gettime(CLOCK_MONOTONIC, &ts);
        ts.tv_nsec -= lastts.tv_nsec;
        if(ts.tv_nsec < 0) {
            ts.tv_nsec += 1000000000;
            --ts.tv_sec;
        }
        assert(ts.tv_nsec >= 0 && ts.tv_nsec < 1000000000);
        assert(ts.tv_sec >= 0);
        lastts = ts;
        time_ns = ts.tv_nsec + ts.tv_sec * 1000000000;
        
        slProcess(time_ns);
    }
    
    exit(EXIT_SUCCESS);
}


void slLogOutputV(char const * format, va_list args)
{
    vprintf(format, args);
}


void slLightOutputInitialize(SLConfiguration const * config)
{
    UNUSED(config);
}


void slLightOutputShutdown(void)
{
}


void slLightOutputShowLights(SLLightData const * lights)
{
    UNUSED(lights);
}


#endif // STAGE_LIGHTS_USE_TARGET_HARNESS


