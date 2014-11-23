
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


#define SL_TARGET_SPI_DEVICE "/dev/spidev1.0"


static int g_slSpiFd = -1;


// This is a CIE even intensity table for converting linear perceived
// brightness values to PWM values (which are linear power output).
// This is basically a correction table to correct for the way the eye
// perceives color.
uint8_t const g_slCieTable[256] = {
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 
    2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 
    3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 
    5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 
    7, 8, 8, 8, 8, 9, 9, 9, 10, 10, 
    10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 
    13, 14, 14, 15, 15, 15, 16, 16, 17, 17, 
    17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 
    22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 
    28, 28, 29, 29, 30, 31, 31, 32, 32, 33, 
    34, 34, 35, 36, 37, 37, 38, 39, 39, 40, 
    41, 42, 43, 43, 44, 45, 46, 47, 47, 48, 
    49, 50, 51, 52, 53, 54, 54, 55, 56, 57, 
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 
    68, 70, 71, 72, 73, 74, 75, 76, 77, 79, 
    80, 81, 82, 83, 85, 86, 87, 88, 90, 91, 
    92, 94, 95, 96, 98, 99, 100, 102, 103, 105, 
    106, 108, 109, 110, 112, 113, 115, 116, 118, 120, 
    121, 123, 124, 126, 128, 129, 131, 132, 134, 136, 
    138, 139, 141, 143, 145, 146, 148, 150, 152, 154, 
    155, 157, 159, 161, 163, 165, 167, 169, 171, 173, 
    175, 177, 179, 181, 183, 185, 187, 189, 191, 193, 
    196, 198, 200, 202, 204, 207, 209, 211, 214, 216, 
    218, 220, 223, 225, 228, 230, 232, 235, 237, 240, 
    242, 245, 247, 250, 252, 255, 
};


int main(int argc, char * argv[])
{
    struct timespec lastts;
    
    clock_gettime(CLOCK_MONOTONIC, &lastts);
    
    slInitialize(argc, argv);

    while(true)
    {
        struct timespec ts;
        uint64_t time_ns;

        clock_gettime(CLOCK_MONOTONIC, &ts);
        time_ns = (uint64_t)(ts.tv_nsec - lastts.tv_nsec) +
            (uint64_t)(ts.tv_sec - lastts.tv_sec) * 1000000000LLU;
        slAssert(time_ns < 0x8000000000000000LLU);
        lastts = ts;
        
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
    struct spi_ioc_transfer xfer;
    int result;
    uint8_t buf[(SL_NUM_LIGHTS_LEFT + SL_NUM_LIGHTS_RIGHT +
        SL_NUM_LIGHTS_OVERHEAD) * 3];
    size_t j = 0;

    for(size_t i = 0; i < SL_NUM_LIGHTS_LEFT; ++i) {
        buf[j++] = g_slCieTable[lights->left[SL_NUM_LIGHTS_LEFT - 1 - i].b];
        buf[j++] = g_slCieTable[lights->left[SL_NUM_LIGHTS_LEFT - 1 - i].r];
        buf[j++] = g_slCieTable[lights->left[SL_NUM_LIGHTS_LEFT - 1 - i].g];
    }
    
    for(size_t i = 0; i < SL_NUM_LIGHTS_RIGHT; ++i) {
        buf[j++] = g_slCieTable[lights->right[i].b];
        buf[j++] = g_slCieTable[lights->right[i].r];
        buf[j++] = g_slCieTable[lights->right[i].g];
    }
    
    for(size_t i = 0; i < SL_NUM_LIGHTS_OVERHEAD; ++i) {
        buf[j++] = g_slCieTable[lights->overhead[i].b];
        buf[j++] = g_slCieTable[lights->overhead[i].r];
        buf[j++] = g_slCieTable[lights->overhead[i].g];
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


