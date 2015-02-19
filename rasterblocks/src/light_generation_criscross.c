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
} RBLightGeneratorCriscross;


void rbLightGenerationCriscrossFree(void * pData);
void rbLightGenerationCriscrossGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationCriscrossAlloc(RBTexture1 * pPalTex,
    size_t numBars, RBTime spawnInterval, RBTime fadeTime)
{
    RBLightGeneratorCriscross * pCriscross =
        (RBLightGeneratorCriscross *)malloc(
            sizeof (RBLightGeneratorCriscross));
    
    pCriscross->genDef.pData = pCriscross;
    pCriscross->genDef.free = rbLightGenerationCriscrossFree;
    pCriscross->genDef.generate = rbLightGenerationCriscrossGenerate;
    pCriscross->pPalTex = pPalTex;
    pCriscross->numBars = numBars;
    rbStartTimer(&pCriscross->spawnTimer, spawnInterval);
    pCriscross->fadeTime = fadeTime;
    pCriscross->pFadeTimers = (RBTimer *)malloc(sizeof (RBTimer) * numBars);
    for(size_t i = 0; i < numBars; ++i) {
        rbStopTimer(&pCriscross->pFadeTimers[i]);
    }
    
    return &pCriscross->genDef;
}


void rbLightGenerationCriscrossFree(void * pData)
{
    RBLightGeneratorCriscross * pCriscross =
        (RBLightGeneratorCriscross *)pData;
    
    free(pCriscross->pFadeTimers);
    free(pCriscross);
}


void rbLightGenerationCriscrossGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorCriscross * pCriscross =
        (RBLightGeneratorCriscross *)pData;
    RBTime fadeTime = pCriscross->fadeTime;
    size_t numBars = pCriscross->numBars;
    RBColor barColors[numBars];
    int32_t spawnCount = rbGetTimerPeriodsAndReset(&pCriscross->spawnTimer);
    RBTime spawnTime =
        (RBTime)(fadeTime * rbClampF(pAnalysis->bassEnergy, 0.0f, 1.0f));
    
    for(int32_t i = 0; i < spawnCount; ++i) {
        size_t bar = rbRandomI(numBars);
        
        if(rbGetTimeLeft(&pCriscross->pFadeTimers[bar]) < spawnTime) {
            rbStartTimer(&pCriscross->pFadeTimers[bar], spawnTime);
        }
    }
    
    for(size_t bar = 0; bar < numBars; ++bar) {
        barColors[bar] = colorct(t1samplc(pCriscross->pPalTex,
            (float)rbGetTimeLeft(&pCriscross->pFadeTimers[bar]) /
                (float)fadeTime));
    }
    
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j,
                barColors[i * numBars / t2getw(pFrame)]);
        }
    }
}


