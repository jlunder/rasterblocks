#include "light_generation.h"

#include "graphics_util.h"


#define RB_VERTICAL_BARS_NUM_BARS 64


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalette;
    size_t numBars;
    RBTimer spawnTimer;
    RBTime fadeTime;
    size_t intensityIndex;
    RBTimer pFadeTimers[RB_VERTICAL_BARS_NUM_BARS];
} RBLightGeneratorVerticalBars;


void rbLightGenerationVerticalBarsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBParameters const * pParameters,
    RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationVerticalBarsAlloc(
    RBTexture1 * pPalette, size_t numBars, RBTime spawnInterval,
    RBTime fadeTime, size_t intensityIndex)
{
    RBLightGeneratorVerticalBars * pVerticalBars =
        (RBLightGeneratorVerticalBars *)malloc(
            sizeof (RBLightGeneratorVerticalBars));
    
    pVerticalBars->genDef.pData = pVerticalBars;
    pVerticalBars->genDef.free = free;
    pVerticalBars->genDef.generate = rbLightGenerationVerticalBarsGenerate;
    pVerticalBars->pPalette = pPalette;
    pVerticalBars->numBars =
        rbClampI(numBars, 0, RB_VERTICAL_BARS_NUM_BARS - 1);
    rbStartTimer(&pVerticalBars->spawnTimer, spawnInterval);
    pVerticalBars->fadeTime = fadeTime;
    if(pVerticalBars->fadeTime <= 0) {
        pVerticalBars->fadeTime = 1;
    }
    pVerticalBars->intensityIndex = intensityIndex;
    for(size_t i = 0; i < numBars; ++i) {
        rbStopTimer(&pVerticalBars->pFadeTimers[i]);
    }
    
    return &pVerticalBars->genDef;
}


void rbLightGenerationVerticalBarsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBParameters const * pParameters,
    RBTexture2 * pFrame)
{
    RBLightGeneratorVerticalBars * pVerticalBars =
        (RBLightGeneratorVerticalBars *)pData;
    RBTime fadeTime = pVerticalBars->fadeTime;
    size_t numBars = pVerticalBars->numBars;
    RBColor barColors[numBars];
    int32_t spawnCount = rbGetTimerPeriodsAndReset(&pVerticalBars->spawnTimer);
    RBTime spawnTime = (RBTime)(fadeTime *
        rbClampF(rbParameterGetF(pParameters, pVerticalBars->intensityIndex,
            0.0f), 0.0f, 1.0f));
    
    UNUSED(pAnalysis);
    
    for(int32_t i = 0; i < spawnCount; ++i) {
        size_t bar;
        bar = rbRandomI(numBars);
        if(rbGetTimeLeft(&pVerticalBars->pFadeTimers[bar]) <
                spawnTime) {
            rbStartTimer(&pVerticalBars->pFadeTimers[bar], spawnTime);
        }
    }
    
    for(size_t bar = 0; bar < numBars; ++bar) {
        barColors[bar] = colorct(
            t1samplc(pVerticalBars->pPalette,
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


