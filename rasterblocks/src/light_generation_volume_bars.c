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
    int32_t width = t2getw(pFrame);
    int32_t height = t2geth(pFrame);
    float bassWidthF = pAnalysis->bassEnergy * width * 0.25f;
    float trebleWidthF = pAnalysis->trebleEnergy * width * 0.25f;
    int32_t bassWidthI = (int32_t)rbClampF(bassWidthF, 0.0f, width / 2);
    int32_t trebleWidthI = (int32_t)rbClampF(trebleWidthF, 0.0f, width / 2);
    
    t2clear(pFrame, colori(0, 0, 0, 0));
    for(int32_t j = 0; j < height / 8; ++j) {
        for(int32_t i = 0; i < bassWidthI; ++i) {
            RBColor c = colorct(t1samplc(pVolumeBars->pLowPalTex,
                (float)(bassWidthF - i) / (float)(width * 0.5f)));
            int32_t x = i;
            int32_t y = j + height / 16;
            
            t2sett(pFrame, x, y, c);
            t2sett(pFrame, width - 1 - x, y, c);
            t2sett(pFrame, x, height - 1 - y, c);
            t2sett(pFrame, width - 1 - x, height - 1 - y, c);
        }
        for(int32_t i = 0; i < trebleWidthI; ++i) {
            RBColor c = colorct(t1samplc(pVolumeBars->pHiPalTex,
                (float)(trebleWidthF - i) / (float)(width * 0.5f)));
            int32_t x = (width / 2) - 1 - i;
            int32_t y = j + height * 5 / 16;
            
            t2sett(pFrame, x, y, c);
            t2sett(pFrame, width - 1 - x, y, c);
            t2sett(pFrame, x, height - 1 - y, c);
            t2sett(pFrame, width - 1 - x, height - 1 - y, c);
        }
    }
}


