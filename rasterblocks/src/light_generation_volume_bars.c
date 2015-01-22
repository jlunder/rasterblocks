#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pLowPalTex;
    RBTexture1 * pHiPalTex;
} RBLightGeneratorVolumeBars;


void rbLightGenerationVolumeBarsFree(void * pData);
void rbLightGenerationVolumeBarsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationVolumeBarsAlloc(RBTexture1 * pLowPalTex,
    RBTexture1 * pHiPalTex)
{
    RBLightGeneratorVolumeBars * pVolumeBars =
        (RBLightGeneratorVolumeBars *)malloc(
            sizeof (RBLightGeneratorVolumeBars));
    
    pVolumeBars->genDef.pData = pVolumeBars;
    pVolumeBars->genDef.free = rbLightGenerationVolumeBarsFree;
    pVolumeBars->genDef.generate = rbLightGenerationVolumeBarsGenerate;
    pVolumeBars->pLowPalTex = pLowPalTex;
    pVolumeBars->pHiPalTex = pHiPalTex;
    
    return &pVolumeBars->genDef;
}


void rbLightGenerationVolumeBarsFree(void * pData)
{
    RBLightGeneratorVolumeBars * pVolumeBars =
        (RBLightGeneratorVolumeBars *)pData;
    
    free(pVolumeBars);
}


void rbLightGenerationVolumeBarsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorVolumeBars * pVolumeBars =
        (RBLightGeneratorVolumeBars *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pVolumeBars);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


