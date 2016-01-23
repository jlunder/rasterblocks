#ifdef RB_USE_SPIDEV_OUTPUT


#include "light_output.h"

#include <sys/ioctl.h>
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
    "sh -c 'grep -q BB-SPIDEV0 /sys/devices/bone_capemgr.9/slots || " \
    "echo BB-SPIDEV0 > /sys/devices/bone_capemgr.9/slots'"
#define RB_TARGET_SPI_DEVICE_STARTUP_WAIT_MS 2000


static void rbLightOutputSpiDevStartSpiDevice(void);


static int g_rbSpiFd = -1;


void rbLightOutputSpiDevInitialize(RBConfiguration const * pConfig)
{
    uint8_t mode = SPI_MODE_0;
    uint8_t lsbFirst = 0;
    uint8_t bitsPerWord = 8;
    uint32_t maxSpeedHz = 500000;
    
    // rbLightOutputInitialize() guarantees no double-init, but...
    rbLightOutputSpiDevShutdown();
    
    UNUSED(pConfig);
    
    rbLightOutputSpiDevStartSpiDevice();
    rbVerify(g_rbSpiFd >= 0);
    
    rbVerify(ioctl(g_rbSpiFd, SPI_IOC_WR_MODE, &mode) >= 0);
    rbVerify(ioctl(g_rbSpiFd, SPI_IOC_WR_LSB_FIRST, &lsbFirst) >= 0);
    rbVerify(ioctl(g_rbSpiFd, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord) >= 0);
    rbVerify(ioctl(g_rbSpiFd, SPI_IOC_WR_MAX_SPEED_HZ, &maxSpeedHz) >= 0);
}


void rbLightOutputSpiDevStartSpiDevice(void)
{
    for(size_t i = 0; ; ++i) {
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
        if(ret != 0) {
            rbWarning("Failed to start up SPI\n");
        }
        
        rbSleep(rbTimeFromMs(RB_TARGET_SPI_DEVICE_STARTUP_WAIT_MS));
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
    size_t const numLightStrings = pFrame->numLightsPerString;
    size_t const numLightsPerString = pFrame->numLightsPerString;
    
    struct spi_ioc_transfer xfer;
    int result;
    uint8_t buf[numLightStrings * numLightsPerString * 3];
    uint8_t * pB = buf;
    RBColor const * pLights = pFrame->data;

    for(size_t j = 0; j < numLightStrings; ++j) {
        rbAssert(pB < buf + LENGTHOF(buf));
        for(size_t i = 0; i < numLightsPerString; ++i) {
            // WS2801 format! SPIDEV doesn't work for WS2812, so.
            *(pB++) = pLights->b;
            *(pB++) = pLights->r;
            *(pB++) = pLights->g;
            ++pLights;
        }
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


#endif
