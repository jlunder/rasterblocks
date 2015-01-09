#ifdef RB_USE_PIXELPUSHER_OUTPUT


#include "light_output.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <errno.h>
#include <unistd.h>


static uint8_t * rbLightOutputPixelPusherEmitPanel(uint8_t * pBuf,
    RBColor const pLights[RB_PANEL_HEIGHT][RB_PANEL_WIDTH]);


static int g_rbPpSocket = -1;
static struct sockaddr_in g_rbPpAddress;
static uint32_t g_rbPpSeqNumber = 0;


void rbLightOutputPixelPusherInitialize(RBConfiguration const * pConfig)
{
    // rbLightOutputInitialize() guarantees no double-init, but...
    rbLightOutputPixelPusherShutdown();
    
    g_rbPpSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    rbVerify(g_rbPpSocket >= 0);
    
    rbZero(&g_rbPpAddress, sizeof g_rbPpAddress);
    
    g_rbPpAddress.sin_family = AF_INET;
    g_rbPpAddress.sin_addr.s_addr = inet_addr(pConfig->lightOutputParam);
    g_rbPpAddress.sin_port = htons(5005);
}


void rbLightOutputPixelPusherShutdown(void)
{
    if(g_rbPpSocket >= 0) {
        close(g_rbPpSocket);
        g_rbPpSocket = -1;
    }
}


void rbLightOutputPixelPusherShowLights(RBRawLightFrame const * pFrame)
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
            pB = rbLightOutputPixelPusherEmitPanel(pB,
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


uint8_t * rbLightOutputPixelPusherEmitPanel(uint8_t * pBuf,
    RBColor const pLights[RB_PANEL_HEIGHT][RB_PANEL_WIDTH])
{
    // This function looks a lot like rbLightOutputSpiDevEmitPanel but the data
    // order is NOT the same!
    RBColor const * pData = pLights[0];
    for(size_t i = 0; i < RB_PANEL_HEIGHT; i += 2) {
        for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            *(pBuf++) = pData->b;
            *(pBuf++) = pData->g;
            *(pBuf++) = pData->r;
            ++pData;
        }
        pData += RB_PANEL_WIDTH;
        for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            --pData;
            *(pBuf++) = pData->b;
            *(pBuf++) = pData->g;
            *(pBuf++) = pData->r;
        }
        pData += RB_PANEL_WIDTH;
    }
    
    return pBuf;
}


#endif
