#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    size_t numGenerators;
    size_t curGenerator;
    size_t lastGenerator;
    RBTimer rotationTimer;
    RBTimer transitionTimer;
    RBLightGenerator * pGenerators[64];
} RBLightGeneratorTimedRotation;


#define RB_ROTATION_TIME_MS 60000
#define RB_TRANSITION_TIME_MS 1000


void rbLightGenerationTimedRotationFree(void * pData);
void rbLightGenerationTimedRotationGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationTimedRotationAlloc(
    RBLightGenerator * pGenerators[], size_t numGenerators)
{
    RBLightGeneratorTimedRotation * pTimedRotation =
        (RBLightGeneratorTimedRotation *)malloc(
            sizeof (RBLightGeneratorTimedRotation));
    
    pTimedRotation->genDef.pData = pTimedRotation;
    pTimedRotation->genDef.free = rbLightGenerationTimedRotationFree;
    pTimedRotation->genDef.generate = rbLightGenerationTimedRotationGenerate;
    
    if(numGenerators > LENGTHOF(pTimedRotation->pGenerators)) {
        pTimedRotation->numGenerators = LENGTHOF(pTimedRotation->pGenerators);
    }
    else {
        pTimedRotation->numGenerators = numGenerators;
    }
    memcpy(pTimedRotation->pGenerators, pGenerators,
        pTimedRotation->numGenerators * sizeof (RBLightGenerator *));
    
    pTimedRotation->curGenerator = rand() % pTimedRotation->numGenerators;
    pTimedRotation->lastGenerator = 0;
    
    rbStartTimer(&pTimedRotation->rotationTimer,
        rbTimeFromMs(RB_ROTATION_TIME_MS));
    rbStopTimer(&pTimedRotation->transitionTimer);
    
    return &pTimedRotation->genDef;
}


void rbLightGenerationTimedRotationFree(void * pData)
{
    RBLightGeneratorTimedRotation * pTimedRotation =
        (RBLightGeneratorTimedRotation *)pData;
    
    for(size_t i = 0; i < pTimedRotation->numGenerators; ++i) {
        rbLightGenerationGeneratorFree(pTimedRotation->pGenerators[i]);
    }
    free(pTimedRotation);
}


void rbLightGenerationTimedRotationGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorTimedRotation * pTimedRotation =
        (RBLightGeneratorTimedRotation *)pData;
    
    if(rbGetTimerPeriodsAndReset(&pTimedRotation->rotationTimer) != 0) {
        pTimedRotation->lastGenerator = pTimedRotation->curGenerator;
        do {
            pTimedRotation->curGenerator =
                rand() % pTimedRotation->numGenerators;
        } while(pTimedRotation->numGenerators > 1 &&
            pTimedRotation->curGenerator == pTimedRotation->lastGenerator);
        rbStartTimer(&pTimedRotation->transitionTimer,
            rbTimeFromMs(RB_TRANSITION_TIME_MS));
    }
    
    if(rbTimerElapsed(&pTimedRotation->transitionTimer)) {
        rbLightGenerationGeneratorGenerate(
            pTimedRotation->pGenerators[pTimedRotation->curGenerator],
            pAnalysis, pFrame);
    }
    else {
        size_t const fWidth = t2getw(pFrame);
        size_t const fHeight = t2geth(pFrame);
        RBTexture2 * pTempTexA = rbTexture2Alloc(fWidth, fHeight);
        RBTexture2 * pTempTexB = rbTexture2Alloc(fWidth, fHeight);
        float alpha = (float)rbGetTimeLeft(&pTimedRotation->transitionTimer) /
            (float)rbTimeFromMs(RB_TRANSITION_TIME_MS);
        
        rbLightGenerationGeneratorGenerate(
            pTimedRotation->pGenerators[pTimedRotation->lastGenerator],
            pAnalysis, pTempTexA);
        rbLightGenerationGeneratorGenerate(
            pTimedRotation->pGenerators[pTimedRotation->curGenerator],
            pAnalysis, pTempTexB);
        
        rbTexture2Mix(pFrame, pTempTexA, 1.0f - alpha, pTempTexB, alpha);
        
        rbTexture2Free(pTempTexA);
        rbTexture2Free(pTempTexB);
    }
}


