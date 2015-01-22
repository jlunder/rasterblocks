#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBLightGenerator * pGenerator;
    RBTexture2 const * pTexture;
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
    
    return &pImageFilter->genDef;
}


void rbLightGenerationImageFilterFree(void * pData)
{
    RBLightGeneratorImageFilter * pImageFilter =
        (RBLightGeneratorImageFilter *)pData;
    
    rbLightGenerationGeneratorFree(pImageFilter->pGenerator);
    free(pImageFilter);
}


void rbLightGenerationImageFilterGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorImageFilter * pImageFilter =
        (RBLightGeneratorImageFilter *)pData;
    size_t const fWidth = t2getw(pFrame);
    size_t const fHeight = t2geth(pFrame);
    float const fwMul = 1.0f / fWidth;
    float const fhMul = 1.0f / fHeight;
    
    rbLightGenerationGeneratorGenerate(pImageFilter->pGenerator, pAnalysis,
        pFrame);
    for(size_t j = 0; j < fHeight; ++j) {
        for(size_t i = 0; i < fWidth; ++i) {
            t2sett(pFrame, i, j, colorct(ctmul(
                colortempc(t2gett(pFrame, i, j)),
                t2samplc(pImageFilter->pTexture,
                    vector2(i * fwMul, j * fhMul)))));
        }
    }
}


