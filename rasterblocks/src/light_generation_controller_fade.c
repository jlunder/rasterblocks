#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    int32_t controllerNum;
    RBLightGenerator * pGenerator;
} RBLightGeneratorControllerFade;


void rbLightGenerationControllerFadeFree(void * pData);
void rbLightGenerationControllerFadeGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationControllerFadeAlloc(
    RBLightGenerator * pGenerator, int32_t controllerNum)
{
    RBLightGeneratorControllerFade * pControllerFade =
        (RBLightGeneratorControllerFade *)malloc(
            sizeof (RBLightGeneratorControllerFade));
    
    pControllerFade->genDef.pData = pControllerFade;
    pControllerFade->genDef.free = rbLightGenerationControllerFadeFree;
    pControllerFade->genDef.generate =
        rbLightGenerationControllerFadeGenerate;
    
    pControllerFade->pGenerator = pGenerator;
    pControllerFade->controllerNum = controllerNum;
    
    return &pControllerFade->genDef;
}


void rbLightGenerationControllerFadeFree(void * pData)
{
    RBLightGeneratorControllerFade * pControllerFade =
        (RBLightGeneratorControllerFade *)pData;
    
    free(pControllerFade);
}


void rbLightGenerationControllerFadeGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorControllerFade * pControllerFade =
        (RBLightGeneratorControllerFade *)pData;
    size_t const w = t2getw(pFrame);
    size_t const h = t2geth(pFrame);
    float a = 0.5 +
        0.5 * pAnalysis->controls.controllers[pControllerFade->controllerNum];
    
    rbLightGenerationGeneratorGenerate(pControllerFade->pGenerator, pAnalysis,
        pFrame);
    for(size_t j = 0; j < h; ++j) {
        for(size_t i = 0; i < w; ++i) {
            t2sett(pFrame, i, j, cscalef(t2gett(pFrame, i, j), a));
        }
    }
}


