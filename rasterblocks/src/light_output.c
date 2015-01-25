#include "light_output.h"

#include "graphics_util.h"


#define RB_NUM_BIAS_FRAMES 8


static RBLightOutput g_rbLightOutput = RBLO_INVALID;

static float g_rbColorTransform[256];

static float g_rbBiasFrame[RB_NUM_BIAS_FRAMES][RB_PANEL_HEIGHT][RB_PANEL_WIDTH];
static size_t g_rbBiasCounter;


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
    
    brightness = pConfig->brightness;
    
    rbInfo("Generating color table for brightness %g\n", brightness);
    for(size_t i = 0; i < LENGTHOF(g_rbCieTable); ++i) {
        g_rbColorTransform[i] = g_rbCieTable[i] * brightness;
    }
    
    for(size_t k = 0; k < RB_NUM_BIAS_FRAMES; ++k) {
        for(size_t j = 0; j < RB_PANEL_HEIGHT; ++j) {
            for(size_t i = 0; i < RB_PANEL_WIDTH; ++i) {
                g_rbBiasFrame[k][j][i] = rbRandomF();
            }
        }
    }
    
    g_rbBiasCounter = 0;
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
    
    g_rbBiasCounter++;
    for(size_t k = 0; k < RB_NUM_PANELS; ++k) {
        size_t bias = (g_rbBiasCounter + k) % RB_NUM_BIAS_FRAMES;
        for(size_t j = 0; j < RB_PANEL_HEIGHT; ++j) {
            for(size_t i = 0; i < RB_PANEL_WIDTH; ++i) {
                tempFrame.data[k][j][i].r = floorf(
                    g_rbColorTransform[pFrame->data[k][j][i].r] +
                        g_rbBiasFrame[bias][j][i]);
                tempFrame.data[k][j][i].g = floorf(
                    g_rbColorTransform[pFrame->data[k][j][i].g] +
                        g_rbBiasFrame[bias][j][i]);
                tempFrame.data[k][j][i].b = floorf(
                    g_rbColorTransform[pFrame->data[k][j][i].b] +
                        g_rbBiasFrame[bias][j][i]);
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


