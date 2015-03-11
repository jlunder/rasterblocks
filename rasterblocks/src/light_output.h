#ifndef LIGHT_OUTPUT_H_INCLUDED
#define LIGHT_OUTPUT_H_INCLUDED


#include "rasterblocks.h"


void rbLightOutputInitialize(RBConfiguration const * pConfig);
void rbLightOutputShutdown(void);
void rbLightOutputShowLights(RBRawLightFrame const * pFrame);

void rbLightOutputOpenGlInitialize(RBConfiguration const * pConfig);
void rbLightOutputOpenGlShutdown(void);
void rbLightOutputOpenGlShowLights(RBRawLightFrame const * pFrame);

void rbLightOutputPixelPusherInitialize(RBConfiguration const * pConfig);
void rbLightOutputPixelPusherShutdown(void);
void rbLightOutputPixelPusherShowLights(RBRawLightFrame const * pFrame);

void rbLightOutputSpiDevInitialize(RBConfiguration const * pConfig);
void rbLightOutputSpiDevShutdown(void);
void rbLightOutputSpiDevShowLights(RBRawLightFrame const * pFrame);

void rbLightOutputPrussInitialize(RBConfiguration const * pConfig);
void rbLightOutputPrussShutdown(void);
void rbLightOutputPrussShowLights(RBRawLightFrame const * pFrame);


#endif // LIGHT_OUTPUT_H_INCLUDED
