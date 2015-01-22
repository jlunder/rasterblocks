#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
} RBLightGeneratorPulsePlasma;


void rbLightGenerationPulsePlasmaFree(void * pData);
void rbLightGenerationPulsePlasmaGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationPulsePlasmaAlloc(RBTexture1 * pPalTex)
{
    RBLightGeneratorPulsePlasma * pPulsePlasma =
        (RBLightGeneratorPulsePlasma *)malloc(
            sizeof (RBLightGeneratorPulsePlasma));
    
    pPulsePlasma->genDef.pData = pPulsePlasma;
    pPulsePlasma->genDef.free = rbLightGenerationPulsePlasmaFree;
    pPulsePlasma->genDef.generate = rbLightGenerationPulsePlasmaGenerate;
    pPulsePlasma->pPalTex = pPalTex;
    
    return &pPulsePlasma->genDef;
}


void rbLightGenerationPulsePlasmaFree(void * pData)
{
    RBLightGeneratorPulsePlasma * pPulsePlasma =
        (RBLightGeneratorPulsePlasma *)pData;
    
    free(pPulsePlasma);
}


void rbLightGenerationPulsePlasmaGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorPulsePlasma * pPulsePlasma =
        (RBLightGeneratorPulsePlasma *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pPulsePlasma);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


