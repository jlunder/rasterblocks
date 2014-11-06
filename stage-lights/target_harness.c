
#ifdef STAGE_LIGHTS_USE_TARGET_HARNESS

#include <time.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <termios.h>
#include <unistd.h>

#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


#include "stage_lights.h"


#define SL_TARGET_SPI_DEVICE "/dev/spidev0.0"


static int g_slSpiFd = -1;


int main(int argc, char * argv[])
{
    struct timespec lastts;
    
    UNUSED(argc);
   	UNUSED(argv);
    
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
    uint8_t mode = SPI_MODE_0;
    uint8_t lsbFirst = 0;
    uint8_t bitsPerWord = 8;
    uint32_t maxSpeedHz = 500000;
    
    UNUSED(config);
    
    // Just in case this is a reinit
    slLightOutputShutdown();
    
    g_slSpiFd = open(SL_TARGET_SPI_DEVICE, O_RDWR);
    if(g_slSpiFd < 0) {
        slFatal("open() failed for SPI device \"%s\": %s",
            SL_TARGET_SPI_DEVICE, strerror(errno));
    }
    slVerify(ioctl(g_slSpiFd, SPI_IOC_WR_MODE, &mode) >= 0);
    slVerify(ioctl(g_slSpiFd, SPI_IOC_WR_LSB_FIRST, &lsbFirst) >= 0);
    slVerify(ioctl(g_slSpiFd, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord) >= 0);
    slVerify(ioctl(g_slSpiFd, SPI_IOC_WR_MAX_SPEED_HZ, &maxSpeedHz) >= 0);
}


void slLightOutputShutdown(void)
{
    if(g_slSpiFd >= 0) {
        close(g_slSpiFd);
        g_slSpiFd = -1;
    }
}


void slLightOutputShowLights(SLLightData const * lights)
{
    struct spi_ioc_transfer	xfer;
    int result;
    uint8_t buf[SL_NUM_LIGHTS * 3];

    for(size_t i = 0; i < SL_NUM_LIGHTS; ++i) {
        buf[i * 3 + 0] = lights->lights[i].b;
        buf[i * 3 + 1] = lights->lights[i].r;
        buf[i * 3 + 2] = lights->lights[i].g;
    }
    
    memset(&xfer, 0, sizeof xfer);
    
    xfer.tx_buf = (unsigned long)buf;
    xfer.len = sizeof buf;
    
    result = ioctl(g_slSpiFd, SPI_IOC_MESSAGE(1), &xfer);
    if(result < 0) {
        slError("SPI IOCTL failed: %s", strerror(result));
    }
}


#endif // STAGE_LIGHTS_USE_TARGET_HARNESS


