#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBLightGenerator * pGenerator;
    RBTexture2 const * pTexture;
    RBTexture2 * pTempTexture;
} RBLightGeneratorImageFilter;


void rbLightGenerationImageFilterFree(void * pData);
void rbLightGenerationImageFilterGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationImageFilterAlloc(
    RBLightGenerator * pGenerator, RBTexture2 const * pTex)
{
    RBLightGeneratorImageFilter * pImageFilter =
        (RBLightGeneratorImageFilter *)malloc(
            sizeof (RBLightGeneratorImageFilter));
    
    pImageFilter->genDef.pData = pImageFilter;
    pImageFilter->genDef.free = rbLightGenerationImageFilterFree;
    pImageFilter->genDef.generate = rbLightGenerationImageFilterGenerate;
    
    pImageFilter->pGenerator = pGenerator;
    pImageFilter->pTexture = pTex;
    pImageFilter->pTempTexture = rbTexture2Alloc(t2getw(pTex), t2geth(pTex));
    
    return &pImageFilter->genDef;
}


void rbLightGenerationImageFilterFree(void * pData)
{
    RBLightGeneratorImageFilter * pImageFilter =
        (RBLightGeneratorImageFilter *)pData;
    
    rbTexture2Free(pImageFilter->pTempTexture);
    free(pImageFilter);
}


void rbLightGenerationImageFilterGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorImageFilter * pImageFilter =
        (RBLightGeneratorImageFilter *)pData;
    RBTexture2 const * pFilterTex = pImageFilter->pTexture;
    RBTexture2 * pTex = pImageFilter->pTempTexture;
    size_t const iWidth = t2getw(pTex);
    size_t const iHeight = t2geth(pTex);
    
    rbLightGenerationGeneratorGenerate(pImageFilter->pGenerator, pAnalysis,
        pTex);
    for(size_t j = 0; j < iHeight; ++j) {
        for(size_t i = 0; i < iWidth; ++i) {
            t2sett(pTex, i, j, cmul(t2gett(pFilterTex, i, j), t2gett(pTex, i, j)));
        }
    }
    t2clear(pFrame, colori(0, 0, 0, 0));
    rbTexture2Blt(pFrame,
        ((int32_t)t2getw(pFrame) - (int32_t)t2getw(pTex)) / 2,
        ((int32_t)t2geth(pFrame) - (int32_t)t2geth(pTex)) / 2,
        t2getw(pTex), t2geth(pTex), pTex, 0, 0);
    
    /*
    size_t const fWidth = t2getw(pFrame);
    size_t const fHeight = t2geth(pFrame);
    float const fwMul = 1.0f / fWidth;
    float const fhMul = 1.0f / fHeight;
    
    for(size_t j = 0; j < fHeight; ++j) {
        for(size_t i = 0; i < fWidth; ++i) {
            t2sett(pFrame, i, j, colorct(ctmul(
                colortempc(t2gett(pFrame, i, j)),
                t2samplc(pImageFilter->pTexture,
                    vector2(i * fwMul, j * fhMul)))));
        }
    }
    */
}


