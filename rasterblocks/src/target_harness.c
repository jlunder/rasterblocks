#ifdef RB_USE_TARGET_HARNESS

#include "rasterblocks.h"

#include "graphics_util.h"

#include <signal.h>


static bool g_rbUserStopRequest = false;


static void rbSigintHandler(int sigNum);


int main(int argc, char * argv[])
{
    signal(SIGINT, rbSigintHandler);
    
    rbInitialize(argc, argv);

    while(!g_rbUserStopRequest)
    {
        rbProcess();
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


