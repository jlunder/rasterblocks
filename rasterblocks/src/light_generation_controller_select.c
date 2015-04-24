#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    size_t numGenerators;
    size_t curGenerator;
    size_t lastGenerator;
    RBTimer transitionTimer;
    int32_t controllerNum;
    RBLightGenerator * pGenerators[64];
} RBLightGeneratorControllerSelect;


#define RB_TRANSITION_TIME_MS 1000


void rbLightGenerationControllerSelectFree(void * pData);
void rbLightGenerationControllerSelectGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationControllerSelectAlloc(
    RBLightGenerator * pGenerators[], size_t numGenerators,
    int32_t controllerNum)
{
    RBLightGeneratorControllerSelect * pControllerSelect =
        (RBLightGeneratorControllerSelect *)malloc(
            sizeof (RBLightGeneratorControllerSelect));
    
    pControllerSelect->genDef.pData = pControllerSelect;
    pControllerSelect->genDef.free = rbLightGenerationControllerSelectFree;
    pControllerSelect->genDef.generate =
        rbLightGenerationControllerSelectGenerate;
    
    if(numGenerators > LENGTHOF(pControllerSelect->pGenerators)) {
        pControllerSelect->numGenerators =
            LENGTHOF(pControllerSelect->pGenerators);
    }
    else {
        pControllerSelect->numGenerators = numGenerators;
    }
    memcpy(pControllerSelect->pGenerators, pGenerators,
        pControllerSelect->numGenerators * sizeof (RBLightGenerator *));
    
    pControllerSelect->curGenerator = 0;
    pControllerSelect->lastGenerator = 0;
    
    rbStopTimer(&pControllerSelect->transitionTimer);
    
    pControllerSelect->controllerNum = controllerNum;
    
    return &pControllerSelect->genDef;
}


void rbLightGenerationControllerSelectFree(void * pData)
{
    RBLightGeneratorControllerSelect * pControllerSelect =
        (RBLightGeneratorControllerSelect *)pData;
    
    for(size_t i = 0; i < pControllerSelect->numGenerators; ++i) {
        rbLightGenerationGeneratorFree(pControllerSelect->pGenerators[i]);
    }
    free(pControllerSelect);
}


void rbLightGenerationControllerSelectGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorControllerSelect * pControllerSelect =
        (RBLightGeneratorControllerSelect *)pData;
    float rawSelection =
        pAnalysis->controls.controllers[pControllerSelect->controllerNum];
    size_t selection =
        ((rawSelection + 1.0f) * pControllerSelect->numGenerators * 0.5f);
    
    if(selection >= pControllerSelect->numGenerators) {
        selection = pControllerSelect->numGenerators - 1;
    }
    
    if(selection != pControllerSelect->curGenerator) {
        pControllerSelect->lastGenerator = pControllerSelect->curGenerator;
        pControllerSelect->curGenerator = selection;
    }
    
    if(rbTimerElapsed(&pControllerSelect->transitionTimer)) {
        rbLightGenerationGeneratorGenerate(
            pControllerSelect->pGenerators[pControllerSelect->curGenerator],
            pAnalysis, pFrame);
    }
    else {
        size_t const fWidth = t2getw(pFrame);
        size_t const fHeight = t2geth(pFrame);
        RBTexture2 * pTempTexA = rbTexture2Alloc(fWidth, fHeight);
        RBTexture2 * pTempTexB = rbTexture2Alloc(fWidth, fHeight);
        uint8_t alpha = 255 - (uint8_t)rbClampF(
            (float)rbGetTimeLeft(&pControllerSelect->transitionTimer) * 256 /
                (float)rbTimeFromMs(RB_TRANSITION_TIME_MS), 0.0f, 255.0f);
        
        rbLightGenerationGeneratorGenerate(
            pControllerSelect->pGenerators[pControllerSelect->lastGenerator],
            pAnalysis, pTempTexA);
        rbLightGenerationGeneratorGenerate(
            pControllerSelect->pGenerators[pControllerSelect->curGenerator],
            pAnalysis, pTempTexB);
        
        rbTexture2Mix(pFrame, pTempTexA, 255 - alpha, pTempTexB, alpha);
        
        rbTexture2Free(pTempTexA);
        rbTexture2Free(pTempTexB);
    }
}


