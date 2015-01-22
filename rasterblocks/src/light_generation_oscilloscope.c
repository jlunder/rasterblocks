#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBColor color;
} RBLightGeneratorOscilloscope;


void rbLightGenerationOscilloscopeFree(void * pData);
void rbLightGenerationOscilloscopeGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationOscilloscopeAlloc(RBColor color)
{
    RBLightGeneratorOscilloscope * pOscilloscope =
        (RBLightGeneratorOscilloscope *)malloc(
            sizeof (RBLightGeneratorOscilloscope));
    
    pOscilloscope->genDef.pData = pOscilloscope;
    pOscilloscope->genDef.free = rbLightGenerationOscilloscopeFree;
    pOscilloscope->genDef.generate = rbLightGenerationOscilloscopeGenerate;
    pOscilloscope->color = color;
    
    return &pOscilloscope->genDef;
}


void rbLightGenerationOscilloscopeFree(void * pData)
{
    RBLightGeneratorOscilloscope * pOscilloscope =
        (RBLightGeneratorOscilloscope *)pData;
    
    free(pOscilloscope);
}


void rbLightGenerationOscilloscopeGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorOscilloscope * pOscilloscope =
        (RBLightGeneratorOscilloscope *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pOscilloscope);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}

