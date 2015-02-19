#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
    size_t nextIndex;
    struct {
        RBVector2 pos;
        RBVector2 v;
        RBTimer timer;
        bool twinkledLastFrame;
    } fireworks[128];
} RBLightGeneratorFireworks;


#define RB_FIREWORKS_TWINKLE_TIME_MS 1000
#define RB_FIREWORKS_SPAWN_COUNT 16
//#define RB_FIREWORKS_INITIAL_V 0.1f
//#define RB_FIREWORKS_A 0.5f
#define RB_FIREWORKS_INITIAL_V 0.0f
#define RB_FIREWORKS_A 0.0f

void rbLightGenerationFireworksFree(void * pData);
void rbLightGenerationFireworksGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationFireworksAlloc(RBTexture1 * pPalTex)
{
    RBLightGeneratorFireworks * pFireworks =
        (RBLightGeneratorFireworks *)malloc(
            sizeof (RBLightGeneratorFireworks));
    
    pFireworks->genDef.pData = pFireworks;
    pFireworks->genDef.free = rbLightGenerationFireworksFree;
    pFireworks->genDef.generate = rbLightGenerationFireworksGenerate;
    pFireworks->pPalTex = pPalTex;
    pFireworks->nextIndex = 0;
    for(size_t i = 0; i < LENGTHOF(pFireworks->fireworks); ++i) {
        rbStopTimer(&pFireworks->fireworks[i].timer);
    }
    
    return &pFireworks->genDef;
}


void rbLightGenerationFireworksFree(void * pData)
{
    RBLightGeneratorFireworks * pFireworks =
        (RBLightGeneratorFireworks *)pData;
    
    free(pFireworks);
}


void rbLightGenerationFireworksGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBTime const twinkleTime =
        rbTimeFromMs(RB_FIREWORKS_TWINKLE_TIME_MS);
    RBLightGeneratorFireworks * pFireworks =
        (RBLightGeneratorFireworks *)pData;
    
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(0, 0, 0, 0));
        }
    }
    
    if(pAnalysis->peakDetected) {
        for(size_t j = 0; j < RB_FIREWORKS_SPAWN_COUNT; ++j) {
            size_t i = pFireworks->nextIndex;
            float dir = rbRandomF() * 2.0f * RB_PI;
            float v = rbRandomF() * RB_FIREWORKS_INITIAL_V;
            pFireworks->fireworks[i].pos = vector2(rbRandomF(), rbRandomF());
            pFireworks->fireworks[i].v = vector2(cosf(dir) * v, sinf(dir) * v);
            rbStartTimer(&pFireworks->fireworks[i].timer,
                rbTimeFromMs(RB_FIREWORKS_TWINKLE_TIME_MS));
            pFireworks->fireworks[i].twinkledLastFrame = false;
            pFireworks->nextIndex = (i + 1) % LENGTHOF(pFireworks->fireworks);
        }
    }
    
    for(size_t i = 0; i < LENGTHOF(pFireworks->fireworks); ++i) {
        RBTime timeLeft = rbGetTimeLeft(&pFireworks->fireworks[i].timer);
        
        if(timeLeft > 0) {
            float p = (float)timeLeft / twinkleTime;
            float dir = rbRandomF() * 2.0f * RB_PI;
            RBVector2 a = v2scale(vector2(cosf(dir), sinf(dir)),
                rbRandomF() * RB_FIREWORKS_A);
            
            p = p * p * p * p;
            
            if(rbRandomF() < p && !pFireworks->fireworks[i].twinkledLastFrame) {
                size_t posX = (size_t)rbClampF(
                    pFireworks->fireworks[i].pos.x * t2getw(pFrame), 0,
                    t2getw(pFrame) - 1);
                size_t posY = (size_t)rbClampF(
                    pFireworks->fireworks[i].pos.y * t2geth(pFrame), 0,
                    t2geth(pFrame) - 1);
                t2sett(pFrame, posX, posY, colori(0, 0, 0, 0));
                pFireworks->fireworks[i].twinkledLastFrame = true;
            }
            else {
                pFireworks->fireworks[i].twinkledLastFrame = false;
            }
            
            pFireworks->fireworks[i].pos = v2add(pFireworks->fireworks[i].pos,
                v2add(v2scale(pFireworks->fireworks[i].v, RB_VIDEO_FRAME_TIME),
                    v2scale(a,
                        0.5f * RB_VIDEO_FRAME_TIME * RB_VIDEO_FRAME_TIME)));
            pFireworks->fireworks[i].v = v2add(pFireworks->fireworks[i].v,
                v2scale(a, RB_VIDEO_FRAME_TIME));
        }
        else {
            rbStopTimer(&pFireworks->fireworks[i].timer);
        }
    }
}


