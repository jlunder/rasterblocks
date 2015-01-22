#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBColor color;
} RBLightGeneratorPulseCheckerboard;


void rbLightGenerationPulseCheckerboardFree(void * pData);
void rbLightGenerationPulseCheckerboardGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationPulseCheckerboardAlloc(RBColor color)
{
    RBLightGeneratorPulseCheckerboard * pPulseCheckerboard =
        (RBLightGeneratorPulseCheckerboard *)malloc(
            sizeof (RBLightGeneratorPulseCheckerboard));
    
    pPulseCheckerboard->genDef.pData = pPulseCheckerboard;
    pPulseCheckerboard->genDef.free = rbLightGenerationPulseCheckerboardFree;
    pPulseCheckerboard->genDef.generate = rbLightGenerationPulseCheckerboardGenerate;
    pPulseCheckerboard->color = color;
    
    return &pPulseCheckerboard->genDef;
}


void rbLightGenerationPulseCheckerboardFree(void * pData)
{
    RBLightGeneratorPulseCheckerboard * pPulseCheckerboard =
        (RBLightGeneratorPulseCheckerboard *)pData;
    
    free(pPulseCheckerboard);
}


void rbLightGenerationPulseCheckerboardGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorPulseCheckerboard * pPulseCheckerboard =
        (RBLightGeneratorPulseCheckerboard *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pPulseCheckerboard);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


