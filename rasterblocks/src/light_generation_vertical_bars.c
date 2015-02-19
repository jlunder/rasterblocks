#include "light_generation.h"

#include "graphics_util.h"


#define RB_VERTICAL_BARS_SPAWN_TIME_MS 6
#define RB_VERTICAL_BARS_FADE_TIME_MS 500
#define RB_VERTICAL_BARS_COUNT 40


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
    RBTimer spawnTimer;
    RBTimer fadeTimers[RB_VERTICAL_BARS_COUNT];
} RBLightGeneratorVerticalBars;


void rbLightGenerationVerticalBarsFree(void * pData);
void rbLightGenerationVerticalBarsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationVerticalBarsAlloc(RBTexture1 * pPalTex)
{
    RBLightGeneratorVerticalBars * pVerticalBars =
        (RBLightGeneratorVerticalBars *)malloc(
            sizeof (RBLightGeneratorVerticalBars));
    
    pVerticalBars->genDef.pData = pVerticalBars;
    pVerticalBars->genDef.free = rbLightGenerationVerticalBarsFree;
    pVerticalBars->genDef.generate = rbLightGenerationVerticalBarsGenerate;
    pVerticalBars->pPalTex = pPalTex;
    rbStartTimer(&pVerticalBars->spawnTimer,
        rbTimeFromMs(RB_VERTICAL_BARS_SPAWN_TIME_MS));
    for(size_t i = 0; i < LENGTHOF(pVerticalBars->fadeTimers); ++i) {
        rbStopTimer(&pVerticalBars->fadeTimers[i]);
    }
    
    return &pVerticalBars->genDef;
}


void rbLightGenerationVerticalBarsFree(void * pData)
{
    RBLightGeneratorVerticalBars * pVerticalBars =
        (RBLightGeneratorVerticalBars *)pData;
    
    free(pVerticalBars);
}


void rbLightGenerationVerticalBarsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorVerticalBars * pVerticalBars =
        (RBLightGeneratorVerticalBars *)pData;
    RBTime fadeTime = rbTimeFromMs(RB_VERTICAL_BARS_FADE_TIME_MS);
    RBColor barColors[RB_VERTICAL_BARS_COUNT];
    int32_t spawnCount = rbGetTimerPeriodsAndReset(&pVerticalBars->spawnTimer);
    RBTime spawnTime = (RBTime)(fadeTime * pAnalysis->bassEnergy);
    
    for(int32_t i = 0; i < spawnCount; ++i) {
        size_t bar = rbRandomI(RB_VERTICAL_BARS_COUNT);
        
        if(rbGetTimeLeft(&pVerticalBars->fadeTimers[bar]) < spawnTime) {
            rbStartTimer(&pVerticalBars->fadeTimers[bar], spawnTime);
        }
    }
    
    for(size_t bar = 0; bar < RB_VERTICAL_BARS_COUNT; ++bar) {
        barColors[bar] = colorct(t1samplc(pVerticalBars->pPalTex,
            (float)rbGetTimeLeft(&pVerticalBars->fadeTimers[bar]) /
                (float)fadeTime));
    }
    
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j,
                barColors[i * RB_VERTICAL_BARS_COUNT / t2getw(pFrame)]);
        }
    }
}


