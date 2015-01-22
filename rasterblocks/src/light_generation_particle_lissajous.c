#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBColor color;
} RBLightGeneratorParticleLissajous;


void rbLightGenerationParticleLissajousFree(void * pData);
void rbLightGenerationParticleLissajousGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationParticleLissajousAlloc(RBColor color)
{
    RBLightGeneratorParticleLissajous * pParticleLissajous =
        (RBLightGeneratorParticleLissajous *)malloc(
            sizeof (RBLightGeneratorParticleLissajous));
    
    pParticleLissajous->genDef.pData = pParticleLissajous;
    pParticleLissajous->genDef.free = rbLightGenerationParticleLissajousFree;
    pParticleLissajous->genDef.generate = rbLightGenerationParticleLissajousGenerate;
    pParticleLissajous->color = color;
    
    return &pParticleLissajous->genDef;
}


void rbLightGenerationParticleLissajousFree(void * pData)
{
    RBLightGeneratorParticleLissajous * pParticleLissajous =
        (RBLightGeneratorParticleLissajous *)pData;
    
    free(pParticleLissajous);
}


void rbLightGenerationParticleLissajousGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorParticleLissajous * pParticleLissajous =
        (RBLightGeneratorParticleLissajous *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pParticleLissajous);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


