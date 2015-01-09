#ifdef RB_USE_SPIDEV_OUTPUT


#include "light_output.h"

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

#include <linux/types.h>
#include <linux/spi/spidev.h>


#define RB_TARGET_MAX_SPI_OPEN_RETRY 5
#define RB_TARGET_SPI_DEVICE "/dev/spidev1.0"
#define RB_TARGET_SPI_DEVICE_STARTUP_COMMAND \
    "echo BB-SPIDEV0 > /sys/devices/bone_capemgr.9/slots"
#define RB_TARGET_SPI_DEVICE_STARTUP_WAIT_NS 2000000000LLU


static void rbLightOutputSpiDevStartSpiDevice(void);
static uint8_t * rbLightOutputSpiDevEmitPanel(uint8_t * pBuf,
    RBColor const pLights[RB_PANEL_HEIGHT][RB_PANEL_WIDTH]);


static int g_rbSpiFd = -1;


void rbLightOutputSpiDevInitialize(RBConfiguration const * pConfig)
{
    uint8_t mode = SPI_MODE_0;
    uint8_t lsbFirst = 0;
    uint8_t bitsPerWord = 8;
    uint32_t maxSpeedHz = 500000;
    
    // rbLightOutputInitialize() guarantees no double-init, but...
    rbLightOutputSpiDevShutdown();
    
    rbLightOutputStartSpiDevice();
    rbVerify(g_rbSpiFd >= 0);
    
    rbVerify(ioctl(g_rbSpiFd, SPI_IOC_WR_MODE, &mode) >= 0);
    rbVerify(ioctl(g_rbSpiFd, SPI_IOC_WR_LSB_FIRST, &lsbFirst) >= 0);
    rbVerify(ioctl(g_rbSpiFd, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord) >= 0);
    rbVerify(ioctl(g_rbSpiFd, SPI_IOC_WR_MAX_SPEED_HZ, &maxSpeedHz) >= 0);
}


void rbLightOutputSpiDevStartSpiDevice(void)
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


void rbLightOutputSpiDevShutdown(void)
{
    if(g_rbSpiFd >= 0) {
        close(g_rbSpiFd);
        g_rbSpiFd = -1;
    }
}


void rbLightOutputSpiDevShowLights(RBRawLightFrame const * pFrame)
{
    struct spi_ioc_transfer xfer;
    int result;
    uint8_t buf[RB_NUM_LIGHTS * 3];
    uint8_t * pB = buf;

    for(size_t i = 0; i < RB_NUM_PANELS; ++i) {
        rbAssert(pB < buf + LENGTHOF(buf));
        pB = rbLightOutputSpiDevEmitPanel(pB, pFrame->data[i]);
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


uint8_t * rbLightOutputSpiDevEmitPanel(uint8_t * pBuf,
    RBColor const pLights[RB_PANEL_HEIGHT][RB_PANEL_WIDTH])
{
    // This function looks a lot like rbLightOutputPixelPusherEmitPanel but the
    // data order is NOT the same!
    RBColor const * pData = pLights[0];
    for(size_t i = 0; i < RB_PANEL_HEIGHT; i += 2) {
        for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            *(pBuf++) = pData->b;
            *(pBuf++) = pData->r;
            *(pBuf++) = pData->g;
            ++pData;
        }
        pData += RB_PANEL_WIDTH;
        for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            --pData;
            *(pBuf++) = pData->b;
            *(pBuf++) = pData->r;
            *(pBuf++) = pData->g;
        }
        pData += RB_PANEL_WIDTH;
    }
    
    return pBuf;
}


#endif
