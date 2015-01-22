#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBLightGenerator * pGenerator;
    RBTexture2 * pTexture;
} RBLightGeneratorRescale;


void rbLightGenerationRescaleFree(void * pData);
void rbLightGenerationRescaleGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationRescaleAlloc(
    RBLightGenerator * pGenerator, size_t srcWidth, size_t srcHeight)
{
    RBLightGeneratorRescale * pRescale =
        (RBLightGeneratorRescale *)malloc(
            sizeof (RBLightGeneratorRescale));
    
    pRescale->genDef.pData = pRescale;
    pRescale->genDef.free = rbLightGenerationRescaleFree;
    pRescale->genDef.generate = rbLightGenerationRescaleGenerate;
    
    pRescale->pGenerator = pGenerator;
    pRescale->pTexture = rbTexture2Alloc(srcWidth, srcHeight);
    
    return &pRescale->genDef;
}


void rbLightGenerationRescaleFree(void * pData)
{
    RBLightGeneratorRescale * pRescale = (RBLightGeneratorRescale *)pData;
    
    rbLightGenerationGeneratorFree(pRescale->pGenerator);
    rbTexture2Free(pRescale->pTexture);
    free(pRescale);
}


void rbLightGenerationRescaleGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorRescale * pRescale = (RBLightGeneratorRescale *)pData;
    
    rbLightGenerationGeneratorGenerate(pRescale->pGenerator, pAnalysis,
        pRescale->pTexture);
    rbTexture2Rescale(pFrame, pRescale->pTexture);
}


