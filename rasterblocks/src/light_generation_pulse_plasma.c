#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
    RBHarmonicPathGenerator paths[4];
} RBLightGeneratorPulsePlasma;


void rbLightGenerationPulsePlasmaFree(void * pData);
void rbLightGenerationPulsePlasmaGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationPulsePlasmaAlloc(RBTexture1 * pPalTex)
{
    RBLightGeneratorPulsePlasma * pPulsePlasma =
        (RBLightGeneratorPulsePlasma *)malloc(
            sizeof (RBLightGeneratorPulsePlasma));
    
    pPulsePlasma->genDef.pData = pPulsePlasma;
    pPulsePlasma->genDef.free = rbLightGenerationPulsePlasmaFree;
    pPulsePlasma->genDef.generate = rbLightGenerationPulsePlasmaGenerate;
    pPulsePlasma->pPalTex = pPalTex;
    
    rbHarmonicPathGeneratorInitialize(&pPulsePlasma->paths[0],
        1.0f / 19.42f, vector2(-1.0f, -1.0f), vector2( 10.0f,   0.0f),
        vector2(10.0f,  2.0f), vector2(3.0f, 3.0f), vector2(0.5f, 0.5f));
    rbHarmonicPathGeneratorInitialize(&pPulsePlasma->paths[1],
        1.0f / 13.84f, vector2( 1.0f, -1.0f), vector2( 40.0f,   0.0f),
        vector2( 5.0f,  5.0f), vector2(0.7f, 1.5f), vector2(0.0f, 0.0f));
    rbHarmonicPathGeneratorInitialize(&pPulsePlasma->paths[2],
        1.0f / 27.07f, vector2( 0.7f, -1.0f), vector2( 50.0f,   0.0f),
        vector2( 2.0f,  0.0f), vector2(12.0f, 1.0f), vector2(1.5f, 0.0f));
    rbHarmonicPathGeneratorInitialize(&pPulsePlasma->paths[3],
        1.0f / 35.30f, vector2( 0.5f, -1.0f), vector2( 50.0f,   0.0f),
        vector2( 2.0f, 15.0f), vector2(3.0f, 0.0f), vector2(1.5f, 0.0f));
    
    return &pPulsePlasma->genDef;
}


void rbLightGenerationPulsePlasmaFree(void * pData)
{
    RBLightGeneratorPulsePlasma * pPulsePlasma =
        (RBLightGeneratorPulsePlasma *)pData;
    
    free(pPulsePlasma);
}


void rbLightGenerationPulsePlasmaGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorPulsePlasma * pPulsePlasma =
        (RBLightGeneratorPulsePlasma *)pData;
    RBVector2 pathPos[LENGTHOF(pPulsePlasma->paths)];
    RBVector2 frameCenter =
        vector2(t2getw(pFrame) * 0.5f, t2geth(pFrame) * 0.5f);
    float pulseScale = 1.0f + 0.2f * (pAnalysis->bassEnergy - 0.5f);
    
    for(size_t k = 0; k < LENGTHOF(pPulsePlasma->paths); ++k) {
        rbHarmonicPathGeneratorUpdate(&pPulsePlasma->paths[k]);
        pathPos[k] = rbHarmonicPathGeneratorPos(&pPulsePlasma->paths[k]);
        pathPos[k] = v2add(v2scale(v2sub(pathPos[k], frameCenter), pulseScale),
             frameCenter);
    }
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            float a = 0.0f;
            
            a += sinf(v2len(v2sub(vector2(i + 0.5f, j + 0.5f), pathPos[0])) *
                (2.0f * RB_PI / 15.0f));
//            a += sinf(v2len(v2sub(vector2(i + 0.5f, j + 0.5f), pathPos[1])) *
//                (2.0f * RB_PI / 20.0f));
            a += sinf(v2len(v2sub(vector2(i + 0.5f, j + 0.5f), pathPos[2])) *
                (2.0f * RB_PI / 19.0f));
            a += sinf(v2len(v2sub(vector2(i + 0.5f, j + 0.5f), pathPos[3])) *
                (2.0f * RB_PI / 13.0f));
            t2sett(pFrame, i, j, colorct(t1samplc(pPulsePlasma->pPalTex,
                (a * (0.5f / 3)) + 0.5f)));
        }
    }
}


