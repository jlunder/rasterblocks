#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pBassPalTex;
    RBTexture1 * pTreblePalTex;
    size_t numBars;
    RBTimer spawnTimer;
    RBTime fadeTime;
    RBTimer * pFadeTimers;
} RBLightGeneratorVerticalBars;


void rbLightGenerationVerticalBarsFree(void * pData);
void rbLightGenerationVerticalBarsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationVerticalBarsAlloc(
    RBTexture1 * pBassPalTex, RBTexture1 * pTreblePalTex,
    size_t numBars, RBTime spawnInterval, RBTime fadeTime)
{
    RBLightGeneratorVerticalBars * pVerticalBars =
        (RBLightGeneratorVerticalBars *)malloc(
            sizeof (RBLightGeneratorVerticalBars));
    
    pVerticalBars->genDef.pData = pVerticalBars;
    pVerticalBars->genDef.free = rbLightGenerationVerticalBarsFree;
    pVerticalBars->genDef.generate = rbLightGenerationVerticalBarsGenerate;
    pVerticalBars->pBassPalTex = pBassPalTex;
    pVerticalBars->pTreblePalTex = pTreblePalTex;
    pVerticalBars->numBars = numBars;
    rbStartTimer(&pVerticalBars->spawnTimer, spawnInterval);
    pVerticalBars->fadeTime = fadeTime;
    if(pVerticalBars->fadeTime <= 0) {
        pVerticalBars->fadeTime = 1;
    }
    pVerticalBars->pFadeTimers = (RBTimer *)malloc(sizeof (RBTimer) * numBars * 2);
    for(size_t i = 0; i < numBars * 2; ++i) {
        rbStopTimer(&pVerticalBars->pFadeTimers[i]);
    }
    
    return &pVerticalBars->genDef;
}


void rbLightGenerationVerticalBarsFree(void * pData)
{
    RBLightGeneratorVerticalBars * pVerticalBars =
        (RBLightGeneratorVerticalBars *)pData;
    
    free(pVerticalBars->pFadeTimers);
    free(pVerticalBars);
}


void rbLightGenerationVerticalBarsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorVerticalBars * pVerticalBars =
        (RBLightGeneratorVerticalBars *)pData;
    RBTime fadeTime = pVerticalBars->fadeTime;
    size_t numBars = pVerticalBars->numBars;
    RBColor barColors[numBars];
    int32_t spawnCount = rbGetTimerPeriodsAndReset(&pVerticalBars->spawnTimer);
    RBTime bassSpawnTime =
        (RBTime)(fadeTime * rbClampF(pAnalysis->bassEnergy, 0.0f, 1.0f));
    RBTime trebleSpawnTime =
        (RBTime)(fadeTime * rbClampF(pAnalysis->bassEnergy, 0.0f, 1.0f));
    
    for(int32_t i = 0; i < spawnCount; ++i) {
        size_t bar;
        
        bar  = rbRandomI(numBars);
        if(rbGetTimeLeft(&pVerticalBars->pFadeTimers[bar * 2 + 0]) <
                bassSpawnTime) {
            rbStartTimer(&pVerticalBars->pFadeTimers[bar * 2 + 0],
                bassSpawnTime);
        }
        
        //bar = rbRandomI(numBars);
        if(rbGetTimeLeft(&pVerticalBars->pFadeTimers[bar * 2 + 1]) <
                trebleSpawnTime) {
            rbStartTimer(&pVerticalBars->pFadeTimers[bar * 2 + 1],
                trebleSpawnTime);
        }
    }
    
    for(size_t bar = 0; bar < numBars; ++bar) {
        barColors[bar] = colorct(
            ctscale(ctadd(
                t1samplc(pVerticalBars->pBassPalTex,
                    (float)rbGetTimeLeft(&pVerticalBars->pFadeTimers[
                        bar * 2 + 0]) / (float)fadeTime),
                t1samplc(pVerticalBars->pTreblePalTex,
                    (float)rbGetTimeLeft(&pVerticalBars->pFadeTimers[
                        bar * 2 + 1]) / (float)fadeTime)), 0.5f));
    }
    
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j,
                barColors[i * numBars / t2getw(pFrame)]);
        }
    }
}


