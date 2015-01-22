#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
} RBLightGeneratorFireworks;


void rbLightGenerationFireworksFree(void * pData);
void rbLightGenerationFireworksGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationFireworksAlloc(RBTexture1 * pPalTex)
{
    RBLightGeneratorFireworks * pFireworks =
        (RBLightGeneratorFireworks *)malloc(
            sizeof (RBLightGeneratorFireworks));
    
    pFireworks->genDef.pData = pFireworks;
    pFireworks->genDef.free = rbLightGenerationFireworksFree;
    pFireworks->genDef.generate = rbLightGenerationFireworksGenerate;
    pFireworks->pPalTex = pPalTex;
    
    return &pFireworks->genDef;
}


void rbLightGenerationFireworksFree(void * pData)
{
    RBLightGeneratorFireworks * pFireworks =
        (RBLightGeneratorFireworks *)pData;
    
    free(pFireworks);
}


void rbLightGenerationFireworksGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorFireworks * pFireworks =
        (RBLightGeneratorFireworks *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pFireworks);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


