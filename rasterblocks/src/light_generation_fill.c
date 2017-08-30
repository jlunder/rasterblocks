#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    size_t colorIndex;
} RBLightGeneratorFill;


void rbLightGenerationFillFree(void * pData);
void rbLightGenerationFillGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBParameters const * pParameters,
    RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationFillAlloc(size_t colorIndex)
{
    RBLightGeneratorFill * pFill =
        (RBLightGeneratorFill *)malloc(
            sizeof (RBLightGeneratorFill));
    
    pFill->genDef.pData = pFill;
    pFill->genDef.free = rbLightGenerationFillFree;
    pFill->genDef.generate = rbLightGenerationFillGenerate;
    pFill->colorIndex = colorIndex;
    
    return &pFill->genDef;
}


void rbLightGenerationFillFree(void * pData)
{
    RBLightGeneratorFill * pFill =
        (RBLightGeneratorFill *)pData;
    
    free(pFill);
}


void rbLightGenerationFillGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBParameters const * pParameters,
    RBTexture2 * pFrame)
{
    RBLightGeneratorFill * pFill =
        (RBLightGeneratorFill *)pData;
    RBColor fillColor = colorf(pParameters->parameters[pFill->colorIndex],
        pParameters->parameters[pFill->colorIndex + 1],
        pParameters->parameters[pFill->colorIndex + 2],
        pParameters->parameters[pFill->colorIndex + 3]);
    
    UNUSED(pAnalysis);
    
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, fillColor);
        }
    }
}


