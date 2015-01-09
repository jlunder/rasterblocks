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


void rbLightOutputInitialize(RBConfiguration const * config)
{
    uint8_t mode = SPI_MODE_0;
    uint8_t lsbFirst = 0;
    uint8_t bitsPerWord = 8;
    uint32_t maxSpeedHz = 500000;
    
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
    
    g_rbPpSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    rbVerify(g_rbPpSocket >= 0);
    
    rbZero(&g_rbPpAddress, sizeof g_rbPpAddress);
    
    g_rbPpAddress.sin_family = AF_INET;
    g_rbPpAddress.sin_addr.s_addr = inet_addr("192.168.2.4");
    g_rbPpAddress.sin_port = htons(5005);
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
    
    if(g_rbPpSocket >= 0) {
        close(g_rbPpSocket);
        g_rbPpSocket = -1;
    }
}


/*
void rbLightOutputShowLights(RBRawLightFrame const * pFrame)
{
    struct spi_ioc_transfer xfer;
    int result;
    uint8_t buf[RB_NUM_LIGHTS * 3];
    uint8_t * pB = buf;

    for(size_t i = 0; i < RB_NUM_PANELS; ++i) {
        rbAssert(pB < buf + LENGTHOF(buf));
        pB = rbLightOutputEmitPanel(pB, pFrame->data[i]);
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


uint8_t * rbLightOutputEmitPanel(uint8_t * pBuf,
    RBColor const pLights[RB_PANEL_HEIGHT][RB_PANEL_WIDTH])
{
    RBColor const * pData = pLights[0];
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
*/


void rbLightOutputShowLights(RBRawLightFrame const * pFrame)
{
    uint8_t buf[4 + 1 + (RB_PANEL_WIDTH * RB_PANEL_HEIGHT) *
        RB_NUM_PANELS_PER_STRING * 3];
    uint8_t * pB;
    
    for(size_t j = 0; j < RB_NUM_STRINGS; ++j) {
        buf[0] = (g_rbPpSeqNumber >> 24) & 0xFF;
        buf[1] = (g_rbPpSeqNumber >> 16) & 0xFF;
        buf[2] = (g_rbPpSeqNumber >> 8) & 0xFF;
        buf[3] = (g_rbPpSeqNumber >> 0) & 0xFF;
        ++g_rbPpSeqNumber;
        buf[4] = (uint8_t)j;
        pB = buf + 5;
        for(size_t i = 0; i < RB_NUM_PANELS_PER_STRING; ++i) {
            rbAssert(pB < buf + LENGTHOF(buf));
            pB = rbLightOutputEmitPanel(pB,
                pFrame->data[j * RB_NUM_PANELS_PER_STRING + i]);
        }
        rbAssert(pB == buf + LENGTHOF(buf));
        
        rbInfo("%02X %02X %02X %02X:%02X:%02X %02X %02X\n",
            buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
        
        rbVerify(sendto(g_rbPpSocket, buf, sizeof buf, 0,
                (struct sockaddr *)&g_rbPpAddress, sizeof g_rbPpAddress) ==
            sizeof buf);
    }
}


uint8_t * rbLightOutputEmitPanel(uint8_t * pBuf,
    RBColor const pLights[RB_PANEL_HEIGHT][RB_PANEL_WIDTH])
{
    RBColor const * pData = pLights[0];
    for(size_t i = 0; i < RB_PANEL_HEIGHT; i += 2) {
        for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            *(pBuf++) = g_rbModifiedCieTable[pData->b];
            *(pBuf++) = g_rbModifiedCieTable[pData->g];
            *(pBuf++) = g_rbModifiedCieTable[pData->r];
            ++pData;
        }
        pData += RB_PANEL_WIDTH;
        for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            --pData;
            *(pBuf++) = g_rbModifiedCieTable[pData->b];
            *(pBuf++) = g_rbModifiedCieTable[pData->g];
            *(pBuf++) = g_rbModifiedCieTable[pData->r];
        }
        pData += RB_PANEL_WIDTH;
    }
    
    return pBuf;
}


#endif // RB_USE_TARGET_HARNESS


