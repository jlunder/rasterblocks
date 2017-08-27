#include "parameter_generation.h"


static RBParameterGenerator * g_rbPCurrentGenerators[RB_NUM_PARAMETERS];
static char g_rbGeneratorParameterNames
    [RB_NUM_PARAMETERS][RB_PARAMETER_NAME_MAX];
static size_t g_rbGeneratorParameterCounts[RB_NUM_PARAMETERS];
static size_t g_rbNextParameter;


static inline void rbParameterGenerationGeneratorFree(
    RBParameterGenerator * pGenerator)
{
    pGenerator->free(pGenerator->pData);
}


void rbParameterGenerationInitialize(RBConfiguration const * pConfig)
{
    (void)pConfig;
    for(size_t i = 0; i < LENGTHOF(g_rbPCurrentGenerators); ++i) {
        g_rbPCurrentGenerators[i] = NULL;
        g_rbGeneratorParameterCounts[i] = 0;
        g_rbGeneratorParameterNames[i][0] = '\0';
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
    if(RB_NUM_PARAMETERS - g_rbNextParameter < numParameters) {
        size_t result = g_rbNextParameter;
        g_rbNextParameter += numParameters;
        return result;
    }
    else {
        return RB_NUM_PARAMETERS;
    }
}


void rbParameterGenerationSetGenerator(RBParameterGenerator * pGenerator,
    size_t parameterIndex, size_t numParameters, char const * name)
{
    rbAssert(parameterIndex <= RB_NUM_PARAMETERS);
    if(RB_NUM_PARAMETERS - parameterIndex < numParameters) {
        if(g_rbPCurrentGenerators[parameterIndex] != NULL) {
            rbParameterGenerationGeneratorFree(
                g_rbPCurrentGenerators[parameterIndex]);
        }
        g_rbPCurrentGenerators[parameterIndex] = pGenerator;
        g_rbGeneratorParameterCounts[parameterIndex] =
            (pGenerator == NULL ? 0 : numParameters);
        snprintf(g_rbGeneratorParameterNames[parameterIndex],
            sizeof g_rbGeneratorParameterNames[parameterIndex], "%s", name);
        for(size_t i = 1; i < numParameters; ++i) {
            if(g_rbPCurrentGenerators[parameterIndex + i] != NULL) {
                rbParameterGenerationGeneratorFree(
                    g_rbPCurrentGenerators[parameterIndex + i]);
            }
            g_rbPCurrentGenerators[parameterIndex] = NULL;
            g_rbGeneratorParameterCounts[parameterIndex] = 0;
            snprintf(g_rbGeneratorParameterNames[parameterIndex + i],
                sizeof g_rbGeneratorParameterNames[parameterIndex + i],
                "%s %u", name, i);
        }
    }
    else {
        rbWarning("Attempting to set generator \"%s\" on parameter %u + %u, "
            "which is outside the valid range. Did we run out of parameters?",
            name, parameterIndex, numParameters);
    }
}


char const * rbParameterGenerationGetName(size_t parameterIndex)
{
    rbAssert(parameterIndex <= RB_NUM_PARAMETERS);
    if(parameterIndex >= RB_NUM_PARAMETERS) {
        return "<INVALID>";
    }
    return g_rbGeneratorParameterNames[parameterIndex];
}


