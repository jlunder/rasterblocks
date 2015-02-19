#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
    size_t numBars;
    RBTimer spawnTimer;
    RBTime fadeTime;
    RBTimer * pFadeTimers;
} RBLightGeneratorVerticalBars;


void rbLightGenerationVerticalBarsFree(void * pData);
void rbLightGenerationVerticalBarsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationVerticalBarsAlloc(RBTexture1 * pPalTex,
    size_t numBars, RBTime spawnInterval, RBTime fadeTime)
{
    RBLightGeneratorVerticalBars * pVerticalBars =
        (RBLightGeneratorVerticalBars *)malloc(
            sizeof (RBLightGeneratorVerticalBars));
    
    pVerticalBars->genDef.pData = pVerticalBars;
    pVerticalBars->genDef.free = rbLightGenerationVerticalBarsFree;
    pVerticalBars->genDef.generate = rbLightGenerationVerticalBarsGenerate;
    pVerticalBars->pPalTex = pPalTex;
    pVerticalBars->numBars = numBars;
    rbStartTimer(&pVerticalBars->spawnTimer, spawnInterval);
    pVerticalBars->fadeTime = fadeTime;
    pVerticalBars->pFadeTimers = (RBTimer *)malloc(sizeof (RBTimer) * numBars);
    for(size_t i = 0; i < numBars; ++i) {
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
    RBTime spawnTime =
        (RBTime)(fadeTime * rbClampF(pAnalysis->bassEnergy, 0.0f, 1.0f));
    
    for(int32_t i = 0; i < spawnCount; ++i) {
        size_t bar = rbRandomI(numBars);
        
        if(rbGetTimeLeft(&pVerticalBars->pFadeTimers[bar]) < spawnTime) {
            rbStartTimer(&pVerticalBars->pFadeTimers[bar], spawnTime);
        }
    }
    
    for(size_t bar = 0; bar < numBars; ++bar) {
        barColors[bar] = colorct(t1samplc(pVerticalBars->pPalTex,
            (float)rbGetTimeLeft(&pVerticalBars->pFadeTimers[bar]) /
                (float)fadeTime));
    }
    
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j,
                barColors[i * numBars / t2getw(pFrame)]);
        }
    }
}


