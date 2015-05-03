#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
    int32_t triggerNum;
    RBTimer flashTimer;
} RBLightGeneratorTriggerFlash;


#define RB_BEAT_FLASH_TIME_MS 500


void rbLightGenerationTriggerFlashFree(void * pData);
void rbLightGenerationTriggerFlashGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationTriggerFlashAlloc(RBTexture1 * pPalTex,
    int32_t triggerNum)
{
    RBLightGeneratorTriggerFlash * pTriggerFlash =
        (RBLightGeneratorTriggerFlash *)malloc(
            sizeof (RBLightGeneratorTriggerFlash));
    
    rbAssert(triggerNum >= 0 && triggerNum < RB_NUM_TRIGGERS);
    
    pTriggerFlash->genDef.pData = pTriggerFlash;
    pTriggerFlash->genDef.free = rbLightGenerationTriggerFlashFree;
    pTriggerFlash->genDef.generate = rbLightGenerationTriggerFlashGenerate;
    pTriggerFlash->pPalTex = pPalTex;
    pTriggerFlash->triggerNum = triggerNum;
    rbStopTimer(&pTriggerFlash->flashTimer);
    
    return &pTriggerFlash->genDef;
}


void rbLightGenerationTriggerFlashFree(void * pData)
{
    RBLightGeneratorTriggerFlash * pTriggerFlash =
        (RBLightGeneratorTriggerFlash *)pData;
    
    free(pTriggerFlash);
}


void rbLightGenerationTriggerFlashGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorTriggerFlash * pTriggerFlash =
        (RBLightGeneratorTriggerFlash *)pData;
    RBTime const flashTime = rbTimeFromMs(RB_BEAT_FLASH_TIME_MS);
    RBColor fillColor;
    
    if(pAnalysis->controls.triggers[pTriggerFlash->triggerNum]) {
        rbStartTimer(&pTriggerFlash->flashTimer, flashTime);
    }
    
    if(!rbTimerElapsed(&pTriggerFlash->flashTimer)) {
        float flashAlpha = (float)rbGetTimeLeft(&pTriggerFlash->flashTimer) /
            (float)flashTime;
        fillColor = colorct(t1samplc(pTriggerFlash->pPalTex,
            flashAlpha * flashAlpha * flashAlpha * flashAlpha));
    }
    else {
        fillColor = colorct(t1sampnc(pTriggerFlash->pPalTex, 0.0f));
    }
    
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, fillColor);
        }
    }
}


