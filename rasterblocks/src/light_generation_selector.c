#include "light_generation.h"

#include "graphics_util.h"


#define RB_COMPOSITOR_NUM_GENERATORS 32


typedef struct
{
    RBLightGenerator genDef;
    size_t numGenerators;
    size_t curGenerator;
    size_t lastGenerator;
    RBTime transitionTime;
    RBTimer transitionTimer;
    size_t selectIndex;
    RBLightGenerator * pGenerators[RB_COMPOSITOR_NUM_GENERATORS];
} RBLightGeneratorParameterSelect;


#define RB_TRANSITION_TIME_MS 1000


void rbLightGenerationParameterSelectFree(void * pData);
void rbLightGenerationParameterSelectGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBParameters const * pParameters,
    RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationSelectorAlloc(
    RBLightGenerator * const * pGenerators, size_t numGenerators,
    RBTime transitionTime, size_t selectIndex)
{
    RBLightGeneratorParameterSelect * pParameterSelect =
        (RBLightGeneratorParameterSelect *)malloc(
            sizeof (RBLightGeneratorParameterSelect));
    
    pParameterSelect->genDef.pData = pParameterSelect;
    pParameterSelect->genDef.free = rbLightGenerationParameterSelectFree;
    pParameterSelect->genDef.generate =
        rbLightGenerationParameterSelectGenerate;
    
    if(numGenerators > LENGTHOF(pParameterSelect->pGenerators)) {
        pParameterSelect->numGenerators =
            LENGTHOF(pParameterSelect->pGenerators);
    }
    else {
        pParameterSelect->numGenerators = numGenerators;
    }
    memcpy(pParameterSelect->pGenerators, pGenerators,
        pParameterSelect->numGenerators * sizeof (RBLightGenerator *));
    
    pParameterSelect->curGenerator = 0;
    pParameterSelect->lastGenerator = 0;
    
    rbStopTimer(&pParameterSelect->transitionTimer);
    pParameterSelect->transitionTime = transitionTime;
    
    if(selectIndex >= RB_NUM_PARAMETERS) {
        pParameterSelect->selectIndex = 0;
    }
    else {
        pParameterSelect->selectIndex = selectIndex;
    }
    
    return &pParameterSelect->genDef;
}


void rbLightGenerationParameterSelectFree(void * pData)
{
    RBLightGeneratorParameterSelect * pParameterSelect =
        (RBLightGeneratorParameterSelect *)pData;
    
    free(pParameterSelect);
}


void rbLightGenerationParameterSelectGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBParameters const * pParameters,
    RBTexture2 * pFrame)
{
    RBLightGeneratorParameterSelect * pParameterSelect =
        (RBLightGeneratorParameterSelect *)pData;
    size_t selection = rbClampI(
        rbParameterGetF(pParameters, pParameterSelect->selectIndex, 0),
        0, pParameterSelect->numGenerators - 1);
    
    if(selection != pParameterSelect->curGenerator) {
        pParameterSelect->lastGenerator = pParameterSelect->curGenerator;
        pParameterSelect->curGenerator = selection;
    }
    
    if(rbTimerElapsed(&pParameterSelect->transitionTimer)) {
        rbLightGenerationGeneratorGenerate(
            pParameterSelect->pGenerators[pParameterSelect->curGenerator],
            pAnalysis, pParameters, pFrame);
    }
    else {
        size_t const fWidth = t2getw(pFrame);
        size_t const fHeight = t2geth(pFrame);
        RBTexture2 * pTempTexA = rbTexture2Alloc(fWidth, fHeight);
        RBTexture2 * pTempTexB = rbTexture2Alloc(fWidth, fHeight);
        uint8_t alpha = 255 - (uint8_t)rbClampF(
            (float)rbGetTimeLeft(&pParameterSelect->transitionTimer) * 256 /
                (float)rbTimeFromMs(RB_TRANSITION_TIME_MS), 0.0f, 255.0f);
        
        rbLightGenerationGeneratorGenerate(
            pParameterSelect->pGenerators[pParameterSelect->lastGenerator],
            pAnalysis, pParameters, pTempTexA);
        rbLightGenerationGeneratorGenerate(
            pParameterSelect->pGenerators[pParameterSelect->curGenerator],
            pAnalysis, pParameters, pTempTexB);
        
        rbTexture2Mix(pFrame, pTempTexA, 255 - alpha, pTempTexB, alpha);
        
        rbTexture2Free(pTempTexA);
        rbTexture2Free(pTempTexB);
    }
}


