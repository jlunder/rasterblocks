#ifdef RB_USE_TARGET_HARNESS

#include "rasterblocks.h"

#include "graphics_util.h"

#include <signal.h>


static bool g_rbUserStopRequest = false;


static void rbSigintHandler(int sigNum);


int main(int argc, char * argv[])
{
    struct timespec lastts;
    
    clock_gettime(CLOCK_MONOTONIC, &lastts);
    
    signal(SIGINT, rbSigintHandler);
    
    rbInitialize(argc, argv);

    while(!g_rbUserStopRequest)
    {
        struct timespec ts;
        uint64_t time_ns;

        clock_gettime(CLOCK_MONOTONIC, &ts);
        time_ns = (uint64_t)(ts.tv_nsec - lastts.tv_nsec) +
            (uint64_t)(ts.tv_sec - lastts.tv_sec) * 1000000000LLU;
        rbAssert(time_ns < 0x8000000000000000LLU);
        lastts = ts;
        
        rbProcess(time_ns);
        // Make sure we reload g_rbUserStopRequest right before looping
        rbMemoryBarrier();
    }
    
    rbShutdown();
    
    exit(EXIT_SUCCESS);
}


void rbSigintHandler(int sigNum)
{
    UNUSED(sigNum);
    if(g_rbUserStopRequest) {
        // Hmm, not shutting down fast enough? User really wants us dead
        abort();
    }
    g_rbUserStopRequest = true;
    rbMemoryBarrier();
}


void rbLogOutputV(char const * format, va_list args)
{
    vprintf(format, args);
}


#endif // RB_USE_TARGET_HARNESS


