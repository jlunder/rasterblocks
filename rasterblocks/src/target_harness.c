
#ifdef RB_USE_TARGET_HARNESS

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


#include "rasterblocks.h"


#define RB_TARGET_MAX_SPI_OPEN_RETRY 5
#define RB_TARGET_SPI_DEVICE "/dev/spidev1.0"
#define RB_TARGET_SPI_DEVICE_STARTUP_COMMAND \
    "echo BB-SPIDEV0 > /sys/devices/bone_capemgr.9/rbots"
#define RB_TARGET_SPI_DEVICE_STARTUP_WAIT_NS 2000000000LLU


static void rbLightOutputStartSpiDevice(void);
static uint8_t * rbLightOutputEmitPanel(uint8_t * pBuf,
    RBPanel const * pLights);


static int g_rbSpiFd = -1;

uint8_t g_rbModifiedCieTable[256];


// This is a CIE even intensity table for converting linear perceived
// brightness values to PWM values (which are linear power output).
// This is basically a correction table to correct for the way the eye
// perceives color.
uint8_t const g_rbCieTable[256] = {
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


void rbLightOutputInitialize(RBConfiguration const * config)
{
    uint8_t mode = SPI_MODE_0;
    uint8_t lsbFirst = 0;
    uint8_t bitsPerWord = 8;
    uint32_t maxSpeedHz = 500000;
    
    UNUSED(config);
    
    // Just in case this is a reinit
    rbLightOutputShutdown();
    
    rbLightOutputStartSpiDevice();
    rbVerify(g_rbSpiFd >= 0);
    
    rbVerify(ioctl(g_rbSpiFd, SPI_IOC_WR_MODE, &mode) >= 0);
    rbVerify(ioctl(g_rbSpiFd, SPI_IOC_WR_LSB_FIRST, &lsbFirst) >= 0);
    rbVerify(ioctl(g_rbSpiFd, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord) >= 0);
    rbVerify(ioctl(g_rbSpiFd, SPI_IOC_WR_MAX_SPEED_HZ, &maxSpeedHz) >= 0);
    
    for(size_t i = 0; i < LENGTHOF(g_rbCieTable); ++i) {
        g_rbModifiedCieTable[i] = (uint8_t)rbClampF(
            ceilf(g_rbCieTable[i] * config->brightness), 0.0f, 255.0f);
    }
}


void rbLightOutputStartSpiDevice(void)
{
    for(size_t i = 0; ; ++i) {
        struct timespec rbeepTs;
        
        g_rbSpiFd = open(RB_TARGET_SPI_DEVICE, O_RDWR);
        
        if(g_rbSpiFd >= 0) {
            // Done! Success.
            return;
        }
        
        if(i >= RB_TARGET_MAX_SPI_OPEN_RETRY) {
            rbFatal("open() failed for SPI device \"%s\": %s",
                RB_TARGET_SPI_DEVICE, strerror(errno));
        }
        
        rbWarning("BB SPI device not found, attempting startup\n");
        int ret = system(RB_TARGET_SPI_DEVICE_STARTUP_COMMAND);
        if(ret==-1) {
            rbWarning("Failed to start up SPI\n");
        }
        
        rbeepTs.tv_sec = RB_TARGET_SPI_DEVICE_STARTUP_WAIT_NS / 1000000000LLU;
        rbeepTs.tv_nsec = RB_TARGET_SPI_DEVICE_STARTUP_WAIT_NS % 1000000000LLU;
        
        nanosleep(&rbeepTs, NULL);
    }
}


void rbLightOutputShutdown(void)
{
    if(g_rbSpiFd >= 0) {
        close(g_rbSpiFd);
        g_rbSpiFd = -1;
    }
}


void rbLightOutputShowLights(RBRawLightFrame const * pFrame)
{
    struct spi_ioc_transfer xfer;
    int result;
    uint8_t buf[RB_NUM_LIGHTS * 3];
    uint8_t * pB = buf;

    for(size_t i = 0; i < RB_NUM_PANELS; ++i) {
        rbAssert(pB < buf + LENGTHOF(buf));
        pB = rbLightOutputEmitPanel(pB, &pFrame->data[i];
    }
    rbAssert(pB == buf + LENGTHOF(buf));

    memset(&xfer, 0, sizeof xfer);
    
    xfer.tx_buf = (unsigned long)buf;
    xfer.len = sizeof buf;
    
    result = ioctl(g_rbSpiFd, SPI_IOC_MESSAGE(1), &xfer);
    if(result < 0) {
        rbError("SPI IOCTL failed: %s", strerror(result));
    }
}


uint8_t * rbLightOutputEmitPanel(uint8_t * pBuf, RBPanel const * pLights)
{
    RBColor const * pData = pLights->data[0];
    for(size_t i = 0; i < RB_PANEL_HEIGHT; i += 2) {
        for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            *(pBuf++) = g_rbModifiedCieTable[pData->b];
            *(pBuf++) = g_rbModifiedCieTable[pData->r];
            *(pBuf++) = g_rbModifiedCieTable[pData->g];
            ++pData;
        }
        pData += RB_PANEL_WIDTH;
        for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            --pData;
            *(pBuf++) = g_rbModifiedCieTable[pData->b];
            *(pBuf++) = g_rbModifiedCieTable[pData->r];
            *(pBuf++) = g_rbModifiedCieTable[pData->g];
        }
        pData += RB_PANEL_WIDTH;
    }
    
    return pBuf;
}


#endif // RB_USE_TARGET_HARNESS


