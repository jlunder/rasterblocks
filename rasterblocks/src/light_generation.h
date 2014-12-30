#ifndef LIGHT_GENERATION_H_INCLUDED
#define LIGHT_GENERATION_H_INCLUDED


#include "rasterblocks.h"


// Light generation subsystem
void rbLightGenerationInitialize(RBConfiguration const * config);
void rbLightGenerationShutdown(void);
void rbLightGenerationGenerate(RBAnalyzedAudio const * analysis,
    RBLightData * lights);


#endif

