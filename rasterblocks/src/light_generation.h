#ifndef LIGHT_GENERATION_H_INCLUDED
#define LIGHT_GENERATION_H_INCLUDED


#include "rasterblocks.h"

#include "graphics_util.h"


// Light generation subsystem
void rbLightGenerationInitialize(RBConfiguration const * pConfig);
void rbLightGenerationShutdown(void);
void rbLightGenerationGenerate(RBAnalyzedAudio const * pAnalysis,
    RBTexture2 * pFrame);


#endif

