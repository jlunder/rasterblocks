#include "light_output.h"

#include "graphics_util.h"


#define RB_NUM_BIAS_FRAMES 8


static RBLightOutput g_rbLightOutput = RBLO_INVALID;

static float g_rbColorTransform[256];

static float g_rbBiasData[RB_NUM_BIAS_FRAMES * RB_MAX_LIGHTS];


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
    case RBLO_PRUSS:
        rbLightOutputPrussInitialize(pConfig);
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
    
    for(size_t i = 0; i < RB_NUM_BIAS_FRAMES * RB_MAX_LIGHTS; ++i) {
        g_rbBiasData[i] = rbRandomF();
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
    case RBLO_PRUSS:
        rbLightOutputPrussShutdown();
        break;
    default:
    	break;
    }
    
    g_rbLightOutput = RBLO_INVALID;
}


void rbLightOutputShowLights(RBRawLightFrame const * pFrame)
{
    size_t const numLights =
        pFrame->numLightsPerString * pFrame->numLightStrings;
    RBRawLightFrame tempFrame;
    size_t biasOffset = rbRandomI(LENGTHOF(g_rbBiasData) - numLights);
    
    tempFrame.numLightStrings = pFrame->numLightStrings;
    tempFrame.numLightsPerString = pFrame->numLightsPerString;
    
    for(size_t i = 0; i < numLights; ++i) {
        float const bias = g_rbBiasData[biasOffset + i];
        tempFrame.data[i].r = floorf(
            g_rbColorTransform[pFrame->data[i].r] + bias);
        tempFrame.data[i].g = floorf(
            g_rbColorTransform[pFrame->data[i].g] + bias);
        tempFrame.data[i].b = floorf(
            g_rbColorTransform[pFrame->data[i].b] + bias);
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
    case RBLO_PRUSS:
        rbLightOutputPrussShowLights(&tempFrame);
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


#ifndef RB_USE_PRUSS_IO
void rbLightOutputPrussInitialize(RBConfiguration const * pConfig)
{
    UNUSED(pConfig);
    rbFatal("PRUSS output not included in this build!\n");
}


void rbLightOutputPrussShutdown(void)
{
}


void rbLightOutputPrussShowLights(RBRawLightFrame const * pFrame)
{
    UNUSED(pFrame);
}
#endif


