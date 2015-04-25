#include "light_generation.h"

#include "graphics_util.h"


#define RB_FIREWORKS_EXPLODE_MS 2000
#define RB_FIREWORKS_TOTAL_MS 5000
#define RB_FIREWORKS_SPAWN_COUNT 20
#define RB_FIREWORKS_INITIAL_V 0.5f
#define RB_FIREWORKS_INITIAL_V_RAND 0.15f
#define RB_FIREWORKS_EXPLODE_V 1.0f
#define RB_FIREWORKS_EXPLODE_V_RAND 2.0f
#define RB_FIREWORKS_G 0.2f
#define RB_FIREWORKS_DRAG 0.1f
#define RB_FIREWORKS_EXPLODE_DRAG 7.0f


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
    int32_t triggerNum;
    size_t nextIndex;
    struct {
        RBVector2 pos;
        RBVector2 v;
        RBTimer timer;
        bool exploded;
    } fireworks[RB_FIREWORKS_SPAWN_COUNT * 20];
} RBLightGeneratorFireworks;


void rbLightGenerationFireworksFree(void * pData);
void rbLightGenerationFireworksGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationFireworksAlloc(RBTexture1 * pPalTex,
    int32_t triggerNum)
{
    RBLightGeneratorFireworks * pFireworks =
        (RBLightGeneratorFireworks *)malloc(
            sizeof (RBLightGeneratorFireworks));
    
    rbAssert(triggerNum >= 0 && (triggerNum + 1) < RB_NUM_TRIGGERS);
    
    pFireworks->genDef.pData = pFireworks;
    pFireworks->genDef.free = rbLightGenerationFireworksFree;
    pFireworks->genDef.generate = rbLightGenerationFireworksGenerate;
    pFireworks->pPalTex = pPalTex;
    pFireworks->triggerNum = triggerNum;
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
    RBTime const explodeTime =
        rbTimeFromMs(RB_FIREWORKS_EXPLODE_MS);
    RBLightGeneratorFireworks * pFireworks =
        (RBLightGeneratorFireworks *)pData;
    int32_t w = t2getw(pFrame);
    int32_t h = t2geth(pFrame);
    int32_t s = rbMinI(w, h);
    float field[w * h];
    float dt = rbGetDeltaTimeSeconds();
    
    if(pAnalysis->controls.triggers[pFireworks->triggerNum]) {
        float dir = ((rbRandomF() - 0.5f) * 0.25f + 0.5f) * RB_PI;
        float v = RB_FIREWORKS_INITIAL_V +
            rbRandomF() * RB_FIREWORKS_INITIAL_V_RAND;
        RBVector2 pos = vector2((rbRandomF() - 0.5f) * 0.5f, 0.0f);
        RBVector2 vv = vector2(cosf(dir) * v, sinf(dir) * v);
        
        for(size_t j = 0; j < RB_FIREWORKS_SPAWN_COUNT; ++j) {
            size_t i = pFireworks->nextIndex;
            pFireworks->fireworks[i].pos = pos;
            pFireworks->fireworks[i].v = vv;
            rbStartTimer(&pFireworks->fireworks[i].timer,
                rbTimeFromMs(RB_FIREWORKS_TOTAL_MS));
            pFireworks->fireworks[i].exploded = false;
            pFireworks->nextIndex = (i + 1) % LENGTHOF(pFireworks->fireworks);
        }
    }
    
    for(size_t i = 0; i < LENGTHOF(pFireworks->fireworks); ++i) {
        RBTime timeLeft = rbGetTimeLeft(&pFireworks->fireworks[i].timer);
        
        if(timeLeft > 0) {
            RBVector2 a = vector2(0.0f, -RB_FIREWORKS_G);
            RBVector2 v = pFireworks->fireworks[i].v;
            float drag;
            bool explode = timeLeft < explodeTime ||
                pAnalysis->controls.triggers[pFireworks->triggerNum + 1];
            
            if(explode && !pFireworks->fireworks[i].exploded) {
                float dir = rbRandomF() * 2.0f * RB_PI;
                v = v2add(v, v2scale(vector2(cosf(dir), sinf(dir)),
                    RB_FIREWORKS_EXPLODE_V +
                        rbRandomF() * RB_FIREWORKS_EXPLODE_V_RAND));
                pFireworks->fireworks[i].exploded = true;
            }
            
            if(pFireworks->fireworks[i].exploded) {
                drag = RB_FIREWORKS_EXPLODE_DRAG;
            }
            else {
                drag = RB_FIREWORKS_DRAG;
            }
            
            // Drag
            a = v2add(a, v2scale(v, -drag * v2len(v)));
            v = v2add(v, v2scale(a, dt));
            
            pFireworks->fireworks[i].pos = v2add(pFireworks->fireworks[i].pos,
                v2scale(pFireworks->fireworks[i].v, RB_VIDEO_FRAME_TIME));
            pFireworks->fireworks[i].v = v;
        }
        else {
            rbStopTimer(&pFireworks->fireworks[i].timer);
        }
    }
    
    rbZero(field, sizeof field);
    for(size_t i = 0; i < LENGTHOF(pFireworks->fireworks); ++i) {
        RBTime t = rbGetTimeLeft(&pFireworks->fireworks[i].timer);
        
        if(t > 0.0f) {
            RBVector2 p = v2add(v2scale(pFireworks->fireworks[i].pos, s),
                vector2(w / 2 - 0.5f, -(h - 0.5f)));
            int32_t x = (int32_t)p.x;
            int32_t y = (int32_t)-p.y;
            float ax = 1.0f - (p.x - (float)x);
            float ay = 1.0f - (-p.y - (float)y);
            float a = 0.0f;
            
            if(!pFireworks->fireworks[i].exploded) {
                a = 1.0f / RB_FIREWORKS_SPAWN_COUNT;
            }
            else if(t > (RBTime)(explodeTime * 0.5f)) {
                a = 0.5f;
            }
            else {
                if(rbRandomF() < (t / (explodeTime * 0.5f))) {
                    a = rbRandomF() * 2.0f;
                }
                else {
                    a = 0.5f * t / (explodeTime * 0.5f);
                }
            }
            
            if((x > 0) && (x < w) && (y > 0) && (y < h)) {
                field[y * w + x] += ax * ay * a;
            }
            if((x + 1 > 0) && (x + 1 < w) && (y > 0) && (y < h)) {
                field[y * w + (x + 1)] += (1.0f - ax) * ay * a;
            }
            if((x > 0) && (x < w) && (y + 1 > 0) && (y + 1 < h)) {
                field[(y + 1) * w + x] += ax * (1.0f - ay) * a;
            }
            if((x + 1 > 0) && (x + 1 < w) && (y + 1 > 0) && (y + 1 < h)) {
                field[(y + 1) * w + (x + 1)] += (1.0f - ax) * (1.0f - ay) * a;
            }
        }
    }
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colorct(t1samplc(pFireworks->pPalTex,
                field[j * w + i])));
        }
    }
}


