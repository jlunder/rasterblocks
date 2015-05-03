#ifdef RB_USE_PIXELPUSHER_OUTPUT


#include "light_output.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <errno.h>
#include <unistd.h>


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
    size_t const numLightStrings = pFrame->numLightStrings;
    size_t const numLightsPerString = pFrame->numLightsPerString;
    uint8_t buf[4 + 1 + numLightsPerString * 3];
    uint8_t * pB;
    RBColor const * pLights = pFrame->data;
    
    for(size_t k = 0; k < 1; ++k) {
        for(size_t j = 0; j < numLightStrings; ++j) {
            buf[0] = (g_rbPpSeqNumber >> 24) & 0xFF;
            buf[1] = (g_rbPpSeqNumber >> 16) & 0xFF;
            buf[2] = (g_rbPpSeqNumber >> 8) & 0xFF;
            buf[3] = (g_rbPpSeqNumber >> 0) & 0xFF;
            ++g_rbPpSeqNumber;
            buf[4] = (uint8_t)(j + k * numLightStrings);
            pB = buf + 5;
            for(size_t i = 0; i < numLightsPerString; ++i) {
                *(pB++) = pLights->b;
                *(pB++) = pLights->g;
                *(pB++) = pLights->r;
                ++pLights;
            }
            rbAssert(pB == buf + LENGTHOF(buf));
        
            rbInfo("%02X %02X %02X %02X:%02X:%02X %02X %02X\n",
                buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
        
            rbVerify(sendto(g_rbPpSocket, buf, sizeof buf, 0,
                    (struct sockaddr *)&g_rbPpAddress, sizeof g_rbPpAddress) ==
                (int)sizeof buf);
        }
        rbAssert((pLights - pFrame->data) < (ssize_t)LENGTHOF(pFrame->data));
    }
}


#endif
