#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalette;
    size_t scaleIndex;
    RBHarmonicPathGenerator paths[4];
} RBLightGeneratorPlasma;


void rbLightGenerationPlasmaFree(void * pData);
void rbLightGenerationPlasmaGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBParameters const * pParameters,
    RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationPlasmaAlloc(RBTexture1 * pPalette,
    size_t scaleIndex)
{
    RBLightGeneratorPlasma * pPlasma =
        (RBLightGeneratorPlasma *)malloc(
            sizeof (RBLightGeneratorPlasma));
    
    pPlasma->genDef.pData = pPlasma;
    pPlasma->genDef.free = rbLightGenerationPlasmaFree;
    pPlasma->genDef.generate = rbLightGenerationPlasmaGenerate;
    pPlasma->pPalette = pPalette;
    pPlasma->scaleIndex = scaleIndex;
    
    rbHarmonicPathGeneratorInitialize(&pPlasma->paths[0],
        1.0f / 19.42f, vector2(-1.0f, -1.0f), vector2( 10.0f,   0.0f),
        vector2(10.0f,  2.0f), vector2(3.0f, 3.0f), vector2(0.5f, 0.5f));
    rbHarmonicPathGeneratorInitialize(&pPlasma->paths[1],
        1.0f / 13.84f, vector2( 1.0f, -1.0f), vector2( 40.0f,   0.0f),
        vector2( 5.0f,  5.0f), vector2(0.7f, 1.5f), vector2(0.0f, 0.0f));
    rbHarmonicPathGeneratorInitialize(&pPlasma->paths[2],
        1.0f / 27.07f, vector2( 0.7f, -1.0f), vector2( 50.0f,   0.0f),
        vector2( 2.0f,  0.0f), vector2(12.0f, 1.0f), vector2(1.5f, 0.0f));
    rbHarmonicPathGeneratorInitialize(&pPlasma->paths[3],
        1.0f / 35.30f, vector2( 0.5f, -1.0f), vector2( 50.0f,   0.0f),
        vector2( 2.0f, 15.0f), vector2(3.0f, 0.0f), vector2(1.5f, 0.0f));
    
    return &pPlasma->genDef;
}


void rbLightGenerationPlasmaFree(void * pData)
{
    RBLightGeneratorPlasma * pPlasma =
        (RBLightGeneratorPlasma *)pData;
    
    free(pPlasma);
}


void rbLightGenerationPlasmaGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBParameters const * pParameters,
    RBTexture2 * pFrame)
{
    RBLightGeneratorPlasma * pPlasma =
        (RBLightGeneratorPlasma *)pData;
    RBTexture1 * pPalette = pPlasma->pPalette;
    RBVector2 pathPos[LENGTHOF(pPlasma->paths)];
    float invScale =
        1.0f / rbParameterGetF(pParameters, pPlasma->scaleIndex, 1.0f);
    
    UNUSED(pAnalysis);
    
    for(size_t k = 0; k < LENGTHOF(pPlasma->paths); ++k) {
        rbHarmonicPathGeneratorUpdate(&pPlasma->paths[k]);
        pathPos[k] = rbHarmonicPathGeneratorPos(&pPlasma->paths[k]);
    }
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            float a = 0.0f;
            RBVector2 pos = vector2((i + 0.5f) * invScale,
                (j + 0.5f) * invScale);
            
            a += sinf(v2len(v2sub(pos, pathPos[0])) * (2.0f * RB_PI / 15.0f));
//            a += sinf(v2len(v2sub(pos, pathPos[1])) *
//                (2.0f * RB_PI / 20.0f));
            a += sinf(v2len(v2sub(pos, pathPos[2])) * (2.0f * RB_PI / 19.0f));
            a += sinf(v2len(v2sub(pos, pathPos[3])) * (2.0f * RB_PI / 13.0f));
            t2sett(pFrame, i, j, colorct(t1samplc(pPalette,
                (a * (0.5f / 3)) + 0.5f)));
        }
    }
}


