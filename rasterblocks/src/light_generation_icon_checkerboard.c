#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBColor color;
} RBLightGeneratorIconCheckerboard;


void rbLightGenerationIconCheckerboardFree(void * pData);
void rbLightGenerationIconCheckerboardGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationIconCheckerboardAlloc(RBColor color)
{
    RBLightGeneratorIconCheckerboard * pIconCheckerboard =
        (RBLightGeneratorIconCheckerboard *)malloc(
            sizeof (RBLightGeneratorIconCheckerboard));
    
    pIconCheckerboard->genDef.pData = pIconCheckerboard;
    pIconCheckerboard->genDef.free = rbLightGenerationIconCheckerboardFree;
    pIconCheckerboard->genDef.generate = rbLightGenerationIconCheckerboardGenerate;
    pIconCheckerboard->color = color;
    
    return &pIconCheckerboard->genDef;
}


void rbLightGenerationIconCheckerboardFree(void * pData)
{
    RBLightGeneratorIconCheckerboard * pIconCheckerboard =
        (RBLightGeneratorIconCheckerboard *)pData;
    
    free(pIconCheckerboard);
}


void rbLightGenerationIconCheckerboardGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorIconCheckerboard * pIconCheckerboard =
        (RBLightGeneratorIconCheckerboard *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pIconCheckerboard);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


