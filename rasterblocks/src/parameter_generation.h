#ifndef PARAMETER_GENERATION_H_INCLUDED
#define PARAMETER_GENERATION_H_INCLUDED


#include "rasterblocks.h"


typedef struct {
    void (*free)(void * pData);
    void (*generate)(void * pData, RBAnalyzedAudio const * pAnalysis,
        float * pParameters, size_t numParameters);
    void * pData;
} RBParameterGenerator;


// Parameter generation subsystem
extern void rbParameterGenerationInitialize(RBConfiguration const * pConfig);
extern void rbParameterGenerationShutdown(void);
extern void rbParameterGenerationGenerate(RBAnalyzedAudio const * pAnalysis,
    RBParameters * pParameters);
extern size_t rbParameterGenerationAllocateParameters(size_t numParameters);
extern void rbParameterGenerationSetGenerator(
    RBParameterGenerator * pGenerator, size_t parameterIndex,
    size_t numParameters);


#endif


