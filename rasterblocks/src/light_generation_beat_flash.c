#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
    RBTimer flashTimer;
} RBLightGeneratorBeatFlash;


#define RB_BEAT_FLASH_TIME_MS 500


void rbLightGenerationBeatFlashFree(void * pData);
void rbLightGenerationBeatFlashGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationBeatFlashAlloc(RBTexture1 * pPalTex)
{
    RBLightGeneratorBeatFlash * pBeatFlash =
        (RBLightGeneratorBeatFlash *)malloc(
            sizeof (RBLightGeneratorBeatFlash));
    
    pBeatFlash->genDef.pData = pBeatFlash;
    pBeatFlash->genDef.free = rbLightGenerationBeatFlashFree;
    pBeatFlash->genDef.generate = rbLightGenerationBeatFlashGenerate;
    pBeatFlash->pPalTex = pPalTex;
    rbStopTimer(&pBeatFlash->flashTimer);
    
    return &pBeatFlash->genDef;
}


void rbLightGenerationBeatFlashFree(void * pData)
{
    RBLightGeneratorBeatFlash * pBeatFlash =
        (RBLightGeneratorBeatFlash *)pData;
    
    free(pBeatFlash);
}


void rbLightGenerationBeatFlashGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorBeatFlash * pBeatFlash =
        (RBLightGeneratorBeatFlash *)pData;
    RBTime const flashTime = rbTimeFromMs(RB_BEAT_FLASH_TIME_MS);
    RBColor fillColor;
    
    if(pAnalysis->peakDetected) {
        rbStartTimer(&pBeatFlash->flashTimer, flashTime);
    }
    
    if(!rbTimerElapsed(&pBeatFlash->flashTimer)) {
        float flashAlpha = (float)rbGetTimeLeft(&pBeatFlash->flashTimer) /
            (float)flashTime;
        fillColor = colorct(t1samplc(pBeatFlash->pPalTex,
            flashAlpha * flashAlpha * flashAlpha * flashAlpha));
    }
    else {
        fillColor = colorct(t1sampnc(pBeatFlash->pPalTex, 0.0f));
    }
    
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, fillColor);
        }
    }
}


