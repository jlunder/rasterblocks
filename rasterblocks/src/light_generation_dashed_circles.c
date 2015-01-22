#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
    float rotation;
} RBLightGeneratorDashedCircles;


void rbLightGenerationDashedCirclesFree(void * pData);
void rbLightGenerationDashedCirclesGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationDashedCirclesAlloc(RBTexture1 * pPalTex)
{
    RBLightGeneratorDashedCircles * pDashedCircles =
        (RBLightGeneratorDashedCircles *)malloc(
            sizeof (RBLightGeneratorDashedCircles));
    
    pDashedCircles->genDef.pData = pDashedCircles;
    pDashedCircles->genDef.free = rbLightGenerationDashedCirclesFree;
    pDashedCircles->genDef.generate = rbLightGenerationDashedCirclesGenerate;
    pDashedCircles->pPalTex = pPalTex;
    pDashedCircles->rotation = 0.0f;
    
    return &pDashedCircles->genDef;
}


void rbLightGenerationDashedCirclesFree(void * pData)
{
    RBLightGeneratorDashedCircles * pDashedCircles =
        (RBLightGeneratorDashedCircles *)pData;
    
    free(pDashedCircles);
}


void rbLightGenerationDashedCirclesGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorDashedCircles * pDashedCircles =
        (RBLightGeneratorDashedCircles *)pData;
    size_t const fWidth = t2getw(pFrame);
    size_t const fHeight = t2geth(pFrame);
    float centerX = (float)(fWidth - 1) * 0.5f;
    float centerY = (float)(fHeight - 1) * 0.5f;
    float scale = 6.0f;
    float dashScale = 3.0f;
    float dashesOnSmallestRing =
        roundf((scale / (2.0f * dashScale)) * (2.0f * RB_PI));
    float const maxRingLcm = 2.0f * 2.0f * 2.0f * 3.0f * 3.0f * 5.0f * 7.0f;
    
    pDashedCircles->rotation = fmodf(pDashedCircles->rotation +
        0.1f * pAnalysis->trebleEnergy / RB_VIDEO_FRAME_RATE, maxRingLcm);
    
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            RBColor c = colorct(t1samplc(pDashedCircles->pPalTex,
                pAnalysis->bassEnergy * 0.5f));
            float dx = (float)i - centerX;
            float dy = (float)j - centerY;
            float dist = sqrtf(dx * dx + dy * dy);
            float u = fmodf(dist, scale);
            float ring = (dist - u) / scale;
            float rotDir = ((int32_t)ring & 1) == 0 ? -1 : 1;
            float distanceAroundRing = (atan2f(dy, dx) + RB_PI) /
                (2.0f * RB_PI) + maxRingLcm +
                rotDir * pDashedCircles->rotation / ring;
            float v =
                fmodf(dashesOnSmallestRing * ring * distanceAroundRing, 1.0f);
            float a = 0.0f;
            
            if(u < 0.5f) {
                a = u * 2.0f;
            }
            else if(u < 1.5f) {
                a = 1.0f;
            }
            else if(u < 2.0f) {
                a = (2.0f - u) * 2.0f;
            }
            
            if(v < (0.5f / dashScale)) {
                a *= 1.0f - (v * dashScale / 0.5f);
            }
            else if(v < 0.5f) {
                a = 0.0f;
            }
            else if(v < 0.5f + (0.5f / dashScale)) {
                a *= ((v - 0.5f) * dashScale / 0.5f);
            }
            
            t2sett(pFrame, i, j, cscalef(c, a));
        }
    }
}


