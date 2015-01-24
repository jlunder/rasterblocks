#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
    RBHarmonicPathGenerator paths[4];
} RBLightGeneratorPlasma;


void rbLightGenerationPlasmaFree(void * pData);
void rbLightGenerationPlasmaGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationPlasmaAlloc(RBTexture1 * pPalTex)
{
    RBLightGeneratorPlasma * pPlasma =
        (RBLightGeneratorPlasma *)malloc(
            sizeof (RBLightGeneratorPlasma));
    
    pPlasma->genDef.pData = pPlasma;
    pPlasma->genDef.free = rbLightGenerationPlasmaFree;
    pPlasma->genDef.generate = rbLightGenerationPlasmaGenerate;
    pPlasma->pPalTex = pPalTex;
    
    return &pPlasma->genDef;
}


void rbLightGenerationPlasmaFree(void * pData)
{
    RBLightGeneratorPlasma * pPlasma =
        (RBLightGeneratorPlasma *)pData;
    
    free(pPlasma);
}


void rbLightGenerationPlasmaGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorPlasma * pPlasma =
        (RBLightGeneratorPlasma *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pPlasma);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


