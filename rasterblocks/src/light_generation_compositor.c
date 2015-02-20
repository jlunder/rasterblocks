#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBLightGenerator * pGenerators[4];
} RBLightGeneratorCompositor;


void rbLightGenerationCompositorFree(void * pData);
void rbLightGenerationCompositorGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationCompositor2Alloc(
    RBLightGenerator * pGenerator0, RBLightGenerator * pGenerator1)
{
    return rbLightGenerationCompositor4Alloc(pGenerator0, pGenerator1, NULL,
        NULL);
}


RBLightGenerator * rbLightGenerationCompositor3Alloc(
    RBLightGenerator * pGenerator0, RBLightGenerator * pGenerator1,
    RBLightGenerator * pGenerator2)
{
    return rbLightGenerationCompositor4Alloc(pGenerator0, pGenerator1,
        pGenerator2, NULL);
}


RBLightGenerator * rbLightGenerationCompositor4Alloc(
    RBLightGenerator * pGenerator0, RBLightGenerator * pGenerator1,
    RBLightGenerator * pGenerator2, RBLightGenerator * pGenerator3)
{
    RBLightGeneratorCompositor * pCompositor =
        (RBLightGeneratorCompositor *)malloc(
            sizeof (RBLightGeneratorCompositor));
    
    pCompositor->genDef.pData = pCompositor;
    pCompositor->genDef.free = rbLightGenerationCompositorFree;
    pCompositor->genDef.generate = rbLightGenerationCompositorGenerate;
    
    rbZero(pCompositor->pGenerators, sizeof pCompositor->pGenerators);
    
    pCompositor->pGenerators[0] = pGenerator0;
    pCompositor->pGenerators[1] = pGenerator1;
    pCompositor->pGenerators[2] = pGenerator2;
    pCompositor->pGenerators[3] = pGenerator3;
    
    return &pCompositor->genDef;
}


void rbLightGenerationCompositorFree(void * pData)
{
    RBLightGeneratorCompositor * pCompositor =
        (RBLightGeneratorCompositor *)pData;
    
    for(size_t i = 0; i < LENGTHOF(pCompositor->pGenerators); ++i) {
        if(pCompositor->pGenerators[i] != NULL) {
            rbLightGenerationGeneratorFree(pCompositor->pGenerators[i]);
        }
    }
    free(pData);
}


void rbLightGenerationCompositorGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorCompositor * pCompositor =
        (RBLightGeneratorCompositor *)pData;
    size_t const fWidth = t2getw(pFrame);
    size_t const fHeight = t2geth(pFrame);
    RBTexture2 * pTempTex = rbTexture2Alloc(fWidth, fHeight);
    
    for(size_t i = 0; i < LENGTHOF(pCompositor->pGenerators); ++i) {
        if(i == 0) {
            rbLightGenerationGeneratorGenerate(pCompositor->pGenerators[i],
                pAnalysis, pFrame);
        }
        else if(pCompositor->pGenerators[i] != NULL) {
            rbLightGenerationGeneratorGenerate(pCompositor->pGenerators[i],
                pAnalysis, pTempTex);
            rbTexture2BltSrcAlpha(pFrame, 0, 0, fWidth, fHeight, pTempTex,
                0, 0);
        }
    }
    
    rbTexture2Free(pTempTex);
}


