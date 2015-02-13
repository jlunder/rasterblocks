#include "light_generation.h"

#include "graphics_util.h"


typedef struct
{
    RBLightGenerator genDef;
    RBColor color;
    size_t nextIndex;
    struct {
        RBVector2 pos;
        RBVector2 v;
        RBTimer timer;
        bool twinkledLastFrame;
    } stars[128];
} RBLightGeneratorBeatStars;


#define RB_BEAT_STARS_TWINKLE_TIME_MS 1000
#define RB_BEAT_STARS_SPAWN_COUNT 16
//#define RB_BEAT_STARS_INITIAL_V 0.1f
//#define RB_BEAT_STARS_A 0.5f
#define RB_BEAT_STARS_INITIAL_V 0.0f
#define RB_BEAT_STARS_A 0.0f

void rbLightGenerationBeatStarsFree(void * pData);
void rbLightGenerationBeatStarsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationBeatStarsAlloc(RBColor color)
{
    RBLightGeneratorBeatStars * pBeatStars =
        (RBLightGeneratorBeatStars *)malloc(
            sizeof (RBLightGeneratorBeatStars));
    
    pBeatStars->genDef.pData = pBeatStars;
    pBeatStars->genDef.free = rbLightGenerationBeatStarsFree;
    pBeatStars->genDef.generate = rbLightGenerationBeatStarsGenerate;
    pBeatStars->color = color;
    pBeatStars->nextIndex = 0;
    for(size_t i = 0; i < LENGTHOF(pBeatStars->stars); ++i) {
        rbStopTimer(&pBeatStars->stars[i].timer);
    }
    
    return &pBeatStars->genDef;
}


void rbLightGenerationBeatStarsFree(void * pData)
{
    RBLightGeneratorBeatStars * pBeatStars =
        (RBLightGeneratorBeatStars *)pData;
    
    free(pBeatStars);
}


void rbLightGenerationBeatStarsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBTime const twinkleTime =
        rbTimeFromMs(RB_BEAT_STARS_TWINKLE_TIME_MS);
    RBLightGeneratorBeatStars * pBeatStars =
        (RBLightGeneratorBeatStars *)pData;
    
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(0, 0, 0, 0));
        }
    }
    
    if(pAnalysis->peakDetected) {
        for(size_t j = 0; j < RB_BEAT_STARS_SPAWN_COUNT; ++j) {
            size_t i = pBeatStars->nextIndex;
            float dir = rbRandomF() * 2.0f * RB_PI;
            float v = rbRandomF() * RB_BEAT_STARS_INITIAL_V;
            pBeatStars->stars[i].pos = vector2(rbRandomF(), rbRandomF());
            pBeatStars->stars[i].v = vector2(cosf(dir) * v, sinf(dir) * v);
            rbStartTimer(&pBeatStars->stars[i].timer,
                rbTimeFromMs(RB_BEAT_STARS_TWINKLE_TIME_MS));
            pBeatStars->stars[i].twinkledLastFrame = false;
            pBeatStars->nextIndex = (i + 1) % LENGTHOF(pBeatStars->stars);
        }
    }
    
    for(size_t i = 0; i < LENGTHOF(pBeatStars->stars); ++i) {
        RBTime timeLeft = rbGetTimeLeft(&pBeatStars->stars[i].timer);
        
        if(timeLeft > 0) {
            float p = (float)timeLeft / twinkleTime;
            float dir = rbRandomF() * 2.0f * RB_PI;
            RBVector2 a = v2scale(vector2(cosf(dir), sinf(dir)),
                rbRandomF() * RB_BEAT_STARS_A);
            
            p = p * p * p * p;
            
            if(rbRandomF() < p && !pBeatStars->stars[i].twinkledLastFrame) {
                size_t posX = (size_t)rbClampF(
                    pBeatStars->stars[i].pos.x * t2getw(pFrame), 0,
                    t2getw(pFrame) - 1);
                size_t posY = (size_t)rbClampF(
                    pBeatStars->stars[i].pos.y * t2geth(pFrame), 0,
                    t2geth(pFrame) - 1);
                t2sett(pFrame, posX, posY, pBeatStars->color);
                pBeatStars->stars[i].twinkledLastFrame = true;
            }
            else {
                pBeatStars->stars[i].twinkledLastFrame = false;
            }
            
            pBeatStars->stars[i].pos = v2add(pBeatStars->stars[i].pos,
                v2add(v2scale(pBeatStars->stars[i].v, RB_VIDEO_FRAME_TIME),
                    v2scale(a,
                        0.5f * RB_VIDEO_FRAME_TIME * RB_VIDEO_FRAME_TIME)));
            pBeatStars->stars[i].v = v2add(pBeatStars->stars[i].v,
                v2scale(a, RB_VIDEO_FRAME_TIME));
        }
        else {
            rbStopTimer(&pBeatStars->stars[i].timer);
        }
    }
}


