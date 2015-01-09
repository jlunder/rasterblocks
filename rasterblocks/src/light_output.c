#include "light_output.h"

#include "graphics_util.h"


static RBLightOutput g_rbLightOutput = RBLO_INVALID;

static uint8_t g_rbColorTransformR[256];
static uint8_t g_rbColorTransformG[256];
static uint8_t g_rbColorTransformB[256];


void rbLightOutputInitialize(RBConfiguration const * pConfig)
{
    float brightness = 1.0f;
    
    // In case of reinit, shut down first
    rbLightOutputShutdown();
    
    g_rbLightOutput = pConfig->lightOutput;
    switch(g_rbLightOutput) {
    case RBLO_OPENGL:
        rbLightOutputOpenGlInitialize(pConfig);
        break;
    case RBLO_SPIDEV:
        rbLightOutputSpiDevInitialize(pConfig);
        break;
    case RBLO_PIXELPUSHER:
        rbLightOutputPixelPusherInitialize(pConfig);
        break;
    default:
        g_rbLightOutput = RBLO_INVALID;
    	rbFatal("Invalid light output type %d\n", pConfig->lightOutput);
    	break;
    }
    
    if(g_rbLightOutput != RBLO_OPENGL) {
        brightness = pConfig->brightness;
    }
    
    rbInfo("Generating color table for brightness %g\n", brightness);
    for(size_t i = 0; i < LENGTHOF(g_rbCieTable); ++i) {
        g_rbColorTransformR[i] = (uint8_t)rbClampF(
            ceilf(g_rbCieTable[i] * brightness), 0.0f, 255.0f);
        g_rbColorTransformG[i] = (uint8_t)rbClampF(
            ceilf(g_rbCieTable[i] * brightness), 0.0f, 255.0f);
        g_rbColorTransformB[i] = (uint8_t)rbClampF(
            ceilf(g_rbCieTable[i] * brightness), 0.0f, 255.0f);
    }
}


void rbLightOutputShutdown(void)
{
    switch(g_rbLightOutput) {
    case RBLO_OPENGL:
        rbLightOutputOpenGlShutdown();
        break;
    case RBLO_SPIDEV:
        rbLightOutputSpiDevShutdown();
        break;
    case RBLO_PIXELPUSHER:
        rbLightOutputPixelPusherShutdown();
        break;
    default:
    	break;
    }
    
    g_rbLightOutput = RBLO_INVALID;
}


void rbLightOutputShowLights(RBRawLightFrame const * pFrame)
{
    RBRawLightFrame tempFrame;
    
    for(size_t k = 0; k < RB_NUM_PANELS; ++k) {
        for(size_t j = 0; j < RB_PANEL_HEIGHT; ++j) {
            for(size_t i = 0; i < RB_PANEL_WIDTH; ++i) {
                tempFrame.data[k][j][i].r =
                    g_rbColorTransformR[pFrame->data[k][j][i].r];
                tempFrame.data[k][j][i].g =
                    g_rbColorTransformG[pFrame->data[k][j][i].g];
                tempFrame.data[k][j][i].b =
                    g_rbColorTransformB[pFrame->data[k][j][i].b];
            }
        }
    }
    
    switch(g_rbLightOutput) {
    case RBLO_OPENGL:
        rbLightOutputOpenGlShowLights(&tempFrame);
        break;
    case RBLO_SPIDEV:
        rbLightOutputSpiDevShowLights(&tempFrame);
        break;
    case RBLO_PIXELPUSHER:
        rbLightOutputPixelPusherShowLights(&tempFrame);
        break;
    default:
    	break;
    }
}


#ifndef RB_USE_OPENGL_OUTPUT
void rbLightOutputOpenGlInitialize(RBConfiguration const * pConfig)
{
    UNUSED(pConfig);
    rbFatal("OpenGL output not included in this build!\n");
}


void rbLightOutputOpenGlShutdown(void)
{
}


void rbLightOutputOpenGlShowLights(RBRawLightFrame const * pFrame)
{
    UNUSED(pFrame);
}
#endif


#ifndef RB_USE_PIXELPUSHER_OUTPUT
void rbLightOutputPixelPusherInitialize(RBConfiguration const * pConfig)
{
    UNUSED(pConfig);
    rbFatal("PixelPusher output not included in this build!\n");
}


void rbLightOutputPixelPusherShutdown(void)
{
}


void rbLightOutputPixelPusherShowLights(RBRawLightFrame const * pFrame)
{
    UNUSED(pFrame);
}
#endif


#ifndef RB_USE_SPIDEV_OUTPUT
void rbLightOutputSpiDevInitialize(RBConfiguration const * pConfig)
{
    UNUSED(pConfig);
    rbFatal("spidev output not included in this build!\n");
}


void rbLightOutputSpiDevShutdown(void)
{
}


void rbLightOutputSpiDevShowLights(RBRawLightFrame const * pFrame)
{
    UNUSED(pFrame);
}
#endif


