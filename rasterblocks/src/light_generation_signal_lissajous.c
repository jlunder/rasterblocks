#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBColor color;
} RBLightGeneratorSignalLissajous;


void rbLightGenerationSignalLissajousFree(void * pData);
void rbLightGenerationSignalLissajousGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationSignalLissajousAlloc(RBColor color)
{
    RBLightGeneratorSignalLissajous * pSignalLissajous =
        (RBLightGeneratorSignalLissajous *)malloc(
            sizeof (RBLightGeneratorSignalLissajous));
    
    pSignalLissajous->genDef.pData = pSignalLissajous;
    pSignalLissajous->genDef.free = rbLightGenerationSignalLissajousFree;
    pSignalLissajous->genDef.generate = rbLightGenerationSignalLissajousGenerate;
    pSignalLissajous->color = color;
    
    return &pSignalLissajous->genDef;
}


void rbLightGenerationSignalLissajousFree(void * pData)
{
    RBLightGeneratorSignalLissajous * pSignalLissajous =
        (RBLightGeneratorSignalLissajous *)pData;
    
    free(pSignalLissajous);
}


void rbLightGenerationSignalLissajousGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorSignalLissajous * pSignalLissajous =
        (RBLightGeneratorSignalLissajous *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pSignalLissajous);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


