#ifndef LIGHT_GENERATION_H_INCLUDED
#define LIGHT_GENERATION_H_INCLUDED


#include "rasterblocks.h"


// Light generation subsystem
void slLightGenerationInitialize(SLConfiguration const * config);
void slLightGenerationShutdown(void);
void slLightGenerationGenerate(SLAnalyzedAudio const * analysis,
    SLLightData * lights);


#endif

