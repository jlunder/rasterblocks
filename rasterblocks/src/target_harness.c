#ifdef RB_USE_TARGET_HARNESS

#include "rasterblocks.h"

#include "graphics_util.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>


int main(int argc, char * argv[])
{
    struct timespec lastts;
    
    clock_gettime(CLOCK_MONOTONIC, &lastts);
    
    rbInitialize(argc, argv);

    while(true)
    {
        struct timespec ts;
        uint64_t time_ns;

        clock_gettime(CLOCK_MONOTONIC, &ts);
        time_ns = (uint64_t)(ts.tv_nsec - lastts.tv_nsec) +
            (uint64_t)(ts.tv_sec - lastts.tv_sec) * 1000000000LLU;
        rbAssert(time_ns < 0x8000000000000000LLU);
        lastts = ts;
        
        rbProcess(time_ns);
    }
    
    exit(EXIT_SUCCESS);
}


void rbLogOutputV(char const * format, va_list args)
{
    vprintf(format, args);
}


#endif // RB_USE_TARGET_HARNESS


