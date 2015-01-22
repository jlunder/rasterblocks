#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
    size_t width;
    size_t height;
    float * pData;
} RBLightGeneratorSmokeSignals;


void rbLightGenerationSmokeSignalsFree(void * pData);
void rbLightGenerationSmokeSignalsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationSmokeSignalsAlloc(RBTexture1 * pPalTex)
{
    RBLightGeneratorSmokeSignals * pSmokeSignals =
        (RBLightGeneratorSmokeSignals *)malloc(
            sizeof (RBLightGeneratorSmokeSignals));
    
    pSmokeSignals->genDef.pData = pSmokeSignals;
    pSmokeSignals->genDef.free = rbLightGenerationSmokeSignalsFree;
    pSmokeSignals->genDef.generate = rbLightGenerationSmokeSignalsGenerate;
    pSmokeSignals->pPalTex = pPalTex;
    
    return &pSmokeSignals->genDef;
}


void rbLightGenerationSmokeSignalsFree(void * pData)
{
    RBLightGeneratorSmokeSignals * pSmokeSignals =
        (RBLightGeneratorSmokeSignals *)pData;
    
    free(pSmokeSignals);
}


void rbLightGenerationSmokeSignalsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorSmokeSignals * pSmokeSignals =
        (RBLightGeneratorSmokeSignals *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pSmokeSignals);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


