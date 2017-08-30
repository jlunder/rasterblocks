#include "parameter_generation.h"


static RBParameterGenerator * g_rbPCurrentGenerators[RB_NUM_PARAMETERS];
static size_t g_rbGeneratorParameterCounts[RB_NUM_PARAMETERS];
static size_t g_rbNextParameter;


static inline void rbParameterGenerationGeneratorFree(
    RBParameterGenerator * pGenerator)
{
    if(pGenerator->free != NULL) {
        pGenerator->free(pGenerator->pData);
    }
}


void rbParameterGenerationInitialize(RBConfiguration const * pConfig)
{
    (void)pConfig;
    for(size_t i = 0; i < LENGTHOF(g_rbPCurrentGenerators); ++i) {
        g_rbPCurrentGenerators[i] = NULL;
        g_rbGeneratorParameterCounts[i] = 0;
    }
    g_rbNextParameter = 0;
}


void rbParameterGenerationShutdown(void)
{
    for(size_t i = 0; i < LENGTHOF(g_rbPCurrentGenerators); ++i) {
        if(g_rbPCurrentGenerators[i] != NULL) {
            rbParameterGenerationGeneratorFree(g_rbPCurrentGenerators[i]);
            g_rbPCurrentGenerators[i] = NULL;
            g_rbGeneratorParameterCounts[i] = 0;
        }
    }
    g_rbNextParameter = 0;
}


void rbParameterGenerationGenerate(RBAnalyzedAudio const * pAnalysis,
    RBParameters * pParameters)
{
    for(size_t i = 0; i < LENGTHOF(g_rbPCurrentGenerators); ) {
        RBParameterGenerator * pGenerator = g_rbPCurrentGenerators[i];
        size_t numParameters = g_rbGeneratorParameterCounts[i];
        if(pGenerator != NULL && numParameters > 0) {
            pGenerator->generate(pGenerator->pData, pAnalysis,
                pParameters->parameters + i, numParameters);
            i += numParameters;
        }
        else {
            ++i;
        }
    }
}


size_t rbParameterGenerationAllocateParameters(size_t numParameters)
{
    if(RB_NUM_PARAMETERS - g_rbNextParameter <= numParameters) {
        size_t result = g_rbNextParameter;
        g_rbNextParameter += numParameters;
        return result;
    }
    else {
        return RB_NUM_PARAMETERS;
    }
}


void rbParameterGenerationSetGenerator(RBParameterGenerator * pGenerator,
    size_t parameterIndex, size_t numParameters)
{
    rbAssert(parameterIndex <= RB_NUM_PARAMETERS);
    if(RB_NUM_PARAMETERS - parameterIndex <= numParameters) {
        if(g_rbPCurrentGenerators[parameterIndex] != NULL) {
            rbParameterGenerationGeneratorFree(
                g_rbPCurrentGenerators[parameterIndex]);
        }
        g_rbPCurrentGenerators[parameterIndex] = pGenerator;
        g_rbGeneratorParameterCounts[parameterIndex] =
            (pGenerator == NULL ? 0 : numParameters);
        for(size_t i = 1; i < numParameters; ++i) {
            if(g_rbPCurrentGenerators[parameterIndex + i] != NULL) {
                rbParameterGenerationGeneratorFree(
                    g_rbPCurrentGenerators[parameterIndex + i]);
            }
            g_rbPCurrentGenerators[parameterIndex + i] = NULL;
            g_rbGeneratorParameterCounts[parameterIndex + i] = 0;
        }
    }
    else {
        rbWarning("Attempting to set generator on parameter %u + %u, "
            "which is outside the valid range. Did we run out of parameters?",
            parameterIndex, numParameters);
    }
}


