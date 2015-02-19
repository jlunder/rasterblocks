#include "light_generation.h"

#include "graphics_util.h"


#define RB_LISSAJOUS_WINDOW 1024


typedef struct
{
    RBLightGenerator genDef;
    RBColor color;
    float bassData[RB_LISSAJOUS_WINDOW];
    float trebleData[RB_LISSAJOUS_WINDOW];
} RBLightGeneratorSignalLissajous;


void rbLightGenerationSignalLissajousFree(void * pData);
void rbLightGenerationSignalLissajousGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationSignalLissajousAlloc(RBColor color)
{
    RBLightGeneratorSignalLissajous * pSignalLissajous =
        (RBLightGeneratorSignalLissajous *)malloc(
            sizeof (RBLightGeneratorSignalLissajous));
    
    pSignalLissajous->genDef.pData = pSignalLissajous;
    pSignalLissajous->genDef.free = rbLightGenerationSignalLissajousFree;
    pSignalLissajous->genDef.generate = rbLightGenerationSignalLissajousGenerate;
    pSignalLissajous->color = color;
    
    return &pSignalLissajous->genDef;
}


void rbLightGenerationSignalLissajousFree(void * pData)
{
    RBLightGeneratorSignalLissajous * pSignalLissajous =
        (RBLightGeneratorSignalLissajous *)pData;
    
    free(pSignalLissajous);
}


void rbLightGenerationSignalLissajousGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorSignalLissajous * pSignalLissajous =
        (RBLightGeneratorSignalLissajous *)pData;
    // scale adjusts brightness for the (guessed) pixel coverage, by pretending
    // the Lissajous figure will describe a circle.. this is very approximate.
    float scale = (RB_PI * (float)t2getw(pFrame) * 0.5f) *
        pAnalysis->totalEnergy / (float)RB_LISSAJOUS_WINDOW;
    RBColor c = cscalef(pSignalLissajous->color, scale);
    int32_t width = t2getw(pFrame);
    int32_t height = t2geth(pFrame);
    float widthF = width;
    float heightF = height;
    
    if(RB_AUDIO_FRAMES_PER_VIDEO_FRAME < RB_LISSAJOUS_WINDOW) {
        memmove(pSignalLissajous->bassData,
            pSignalLissajous->bassData + RB_AUDIO_FRAMES_PER_VIDEO_FRAME,
            (sizeof *pSignalLissajous->bassData) * (RB_LISSAJOUS_WINDOW -
                RB_AUDIO_FRAMES_PER_VIDEO_FRAME));
        memcpy(pSignalLissajous->bassData + RB_LISSAJOUS_WINDOW -
                RB_AUDIO_FRAMES_PER_VIDEO_FRAME,
            pAnalysis->bassAudio,
            (sizeof *pSignalLissajous->bassData) *
                RB_AUDIO_FRAMES_PER_VIDEO_FRAME);
        memmove(pSignalLissajous->trebleData,
            pSignalLissajous->trebleData + RB_AUDIO_FRAMES_PER_VIDEO_FRAME,
            (sizeof *pSignalLissajous->trebleData) * (RB_LISSAJOUS_WINDOW -
                RB_AUDIO_FRAMES_PER_VIDEO_FRAME));
        memcpy(pSignalLissajous->trebleData + RB_LISSAJOUS_WINDOW -
                RB_AUDIO_FRAMES_PER_VIDEO_FRAME,
            pAnalysis->trebleAudio,
            (sizeof *pSignalLissajous->trebleData) * 
                RB_AUDIO_FRAMES_PER_VIDEO_FRAME);
    }
    else {
        memcpy(pSignalLissajous->bassData,
            pAnalysis->bassAudio + RB_AUDIO_FRAMES_PER_VIDEO_FRAME -
                RB_LISSAJOUS_WINDOW,
            (sizeof *pSignalLissajous->bassData) * RB_LISSAJOUS_WINDOW);
        memcpy(pSignalLissajous->trebleData,
            pAnalysis->trebleAudio + RB_AUDIO_FRAMES_PER_VIDEO_FRAME -
                RB_LISSAJOUS_WINDOW,
            (sizeof *pSignalLissajous->trebleData) * RB_LISSAJOUS_WINDOW);
    }
    
    t2clear(pFrame, colori(0, 0, 0, 0));
    for(size_t i = 0; i < RB_LISSAJOUS_WINDOW; ++i) {
        RBVector2 p = v2rotscale(
            vector2(pSignalLissajous->bassData[i],
                pSignalLissajous->trebleData[i]),
            vector2(1.0f, 0.0f));//vector2(RB_SQRT_2 * 0.5f, -RB_SQRT_2 * 0.5f));
        int32_t x = (int32_t)roundf((p.x * 0.5f + 0.5f) * widthF);
        int32_t y = (int32_t)roundf((p.y * 0.5f + 0.5f) * heightF);
        if(x >= 0 && x < width && y >= 0 && y < height) {
            t2sett(pFrame, x, y, cadd(c, t2gett(pFrame, x, y)));
        }
    }
}


