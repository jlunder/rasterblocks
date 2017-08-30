#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBTexture2 const * pTexture;
} RBLightGeneratorStaticImage;


void rbLightGenerationStaticImageGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBParameters const * pParameters,
    RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationStaticImageAlloc(RBTexture2 const * pTex)
{
    RBLightGeneratorStaticImage * pStaticImage =
        (RBLightGeneratorStaticImage *)malloc(
            sizeof (RBLightGeneratorStaticImage));
    
    pStaticImage->genDef.pData = pStaticImage;
    pStaticImage->genDef.free = free;
    pStaticImage->genDef.generate = rbLightGenerationStaticImageGenerate;
    
    pStaticImage->pTexture = pTex;
    
    return &pStaticImage->genDef;
}


void rbLightGenerationStaticImageGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBParameters const * pParameters,
    RBTexture2 * pFrame)
{
    RBLightGeneratorStaticImage * pStaticImage =
        (RBLightGeneratorStaticImage *)pData;
    RBTexture2 const * pTex = pStaticImage->pTexture;
    
    UNUSED(pAnalysis);
    UNUSED(pParameters);
    
    t2clear(pFrame, colori(0, 0, 0, 0));
    rbTexture2Blt(pFrame,
        ((int32_t)t2getw(pFrame) - (int32_t)t2getw(pTex)) / 2,
        ((int32_t)t2geth(pFrame) - (int32_t)t2geth(pTex)) / 2,
        t2getw(pTex), t2geth(pTex), pTex, 0, 0);
}


