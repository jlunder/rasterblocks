#include "light_generation.h"

#include "graphics_util.h"


// _OSCILLOSCOPE_SAMPLE_RATE is not exact, it'll be rounded to some simple
// fraction of the _AUDIO_SAMPLE_RATE -- _SAMPLE_SKIP is the exact value.
#define RB_OSCILLOSCOPE_SAMPLE_RATE 5000
#define RB_OSCILLOSCOPE_SAMPLE_SKIP \
    ((RB_AUDIO_SAMPLE_RATE + RB_OSCILLOSCOPE_SAMPLE_RATE / 2) / \
        RB_OSCILLOSCOPE_SAMPLE_RATE)
#define RB_OSCILLOSCOPE_WINDOW 1024
#define RB_OSCILLOSCOPE_DISPLAY_WINDOW 256
#define RB_OSCILLOSCOPE_ANTIALIAS 4


typedef struct
{
    RBLightGenerator genDef;
    RBColor color;
    size_t sampleSkipRemainder;
    float data[RB_OSCILLOSCOPE_WINDOW];
} RBLightGeneratorOscilloscope;


void rbLightGenerationOscilloscopeFree(void * pData);
void rbLightGenerationOscilloscopeGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationOscilloscopeAlloc(RBColor color)
{
    RBLightGeneratorOscilloscope * pOscilloscope =
        (RBLightGeneratorOscilloscope *)malloc(
            sizeof (RBLightGeneratorOscilloscope));
    
    pOscilloscope->genDef.pData = pOscilloscope;
    pOscilloscope->genDef.free = rbLightGenerationOscilloscopeFree;
    pOscilloscope->genDef.generate = rbLightGenerationOscilloscopeGenerate;
    pOscilloscope->color = color;
    pOscilloscope->sampleSkipRemainder = 0;
    rbZero(pOscilloscope->data, sizeof pOscilloscope->data);
    
    return &pOscilloscope->genDef;
}


void rbLightGenerationOscilloscopeFree(void * pData)
{
    RBLightGeneratorOscilloscope * pOscilloscope =
        (RBLightGeneratorOscilloscope *)pData;
    
    free(pOscilloscope);
}


void rbLightGenerationOscilloscopeGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorOscilloscope * pOscilloscope =
        (RBLightGeneratorOscilloscope *)pData;
    RBColor c = cscalef(pOscilloscope->color, 1.0f / RB_OSCILLOSCOPE_ANTIALIAS);
    size_t displayStart =
        RB_OSCILLOSCOPE_WINDOW - RB_OSCILLOSCOPE_DISPLAY_WINDOW;
    size_t newDataPoints = (RB_AUDIO_FRAMES_PER_VIDEO_FRAME -
            pOscilloscope->sampleSkipRemainder +
        (RB_OSCILLOSCOPE_SAMPLE_SKIP - 1)) / RB_OSCILLOSCOPE_SAMPLE_SKIP;
    
    if(newDataPoints < RB_OSCILLOSCOPE_WINDOW) {
        size_t inSample = pOscilloscope->sampleSkipRemainder;
        size_t dataSample = RB_OSCILLOSCOPE_WINDOW - newDataPoints;
        
        memmove(pOscilloscope->data, pOscilloscope->data + newDataPoints,
            (sizeof *pOscilloscope->data) * (RB_OSCILLOSCOPE_WINDOW -
                newDataPoints));
        for(; dataSample < RB_OSCILLOSCOPE_WINDOW;
                ++dataSample, inSample += RB_OSCILLOSCOPE_SAMPLE_SKIP) {
            rbAssert(inSample < RB_AUDIO_FRAMES_PER_VIDEO_FRAME);
            pOscilloscope->data[dataSample] = pAnalysis->bassAudio[inSample];
        }
        
        pOscilloscope->sampleSkipRemainder =
            inSample - RB_AUDIO_FRAMES_PER_VIDEO_FRAME;
        
        // Check math...
        rbAssert(pOscilloscope->sampleSkipRemainder <=
            RB_OSCILLOSCOPE_SAMPLE_SKIP);
    }
    else {
        for(size_t dataSample = 0,
                    inSample = RB_AUDIO_FRAMES_PER_VIDEO_FRAME -
                        RB_OSCILLOSCOPE_WINDOW * RB_OSCILLOSCOPE_SAMPLE_SKIP;
                dataSample < RB_OSCILLOSCOPE_WINDOW;
                ++dataSample, inSample += RB_OSCILLOSCOPE_SAMPLE_SKIP) {
            rbAssert(inSample < RB_AUDIO_FRAMES_PER_VIDEO_FRAME);
            pOscilloscope->data[dataSample] = pAnalysis->bassAudio[inSample];
        }
        pOscilloscope->sampleSkipRemainder = 0;
    }
    
    
    for(size_t i = RB_OSCILLOSCOPE_WINDOW - RB_OSCILLOSCOPE_DISPLAY_WINDOW / 2;
            i >= RB_OSCILLOSCOPE_DISPLAY_WINDOW / 2; --i) {
        if(pOscilloscope->data[i] > 0.0f &&
                pOscilloscope->data[i - 1] < 0.0f) {
            displayStart = i - RB_OSCILLOSCOPE_DISPLAY_WINDOW / 2;
            rbInfo("found trigger at %zu, displayStart = %zu\n", i, displayStart);
            break;
        }
    }
    
    rbTexture2Clear(pFrame, colori(0, 0, 0, 0));
    {
        size_t i = displayStart;
        size_t width = t2getw(pFrame);
        int32_t x = 0;
        int32_t height = t2geth(pFrame);
        
        if(width * RB_OSCILLOSCOPE_ANTIALIAS >
                RB_OSCILLOSCOPE_DISPLAY_WINDOW) {
            width = RB_OSCILLOSCOPE_DISPLAY_WINDOW / RB_OSCILLOSCOPE_ANTIALIAS;
            x = (t2getw(pFrame) - width) / 2;
        }
        else {
            i = displayStart + (RB_OSCILLOSCOPE_DISPLAY_WINDOW -
                width * RB_OSCILLOSCOPE_ANTIALIAS) / 2;
        }
        rbAssert(i + width * RB_OSCILLOSCOPE_ANTIALIAS <=
            RB_OSCILLOSCOPE_WINDOW);
        for(size_t j = 0; j < width;
                ++j, i += RB_OSCILLOSCOPE_ANTIALIAS, ++x) {
            for(size_t k = 0; k < RB_OSCILLOSCOPE_ANTIALIAS; ++k) {
                float dA = pOscilloscope->data[i + k];
                float dB;
                float dMin;
                float dMax;
                int32_t yMin;
                int32_t yMax;
                
                if(i + k + 1 < RB_OSCILLOSCOPE_WINDOW) {
                    dB = pOscilloscope->data[i + k + 1];
                }
                else {
                    dB = dA;
                }
                
                if(dA < dB) {
                    dMin = dA;
                    dMax = dB;
                }
                else {
                    dMin = dB;
                    dMax = dA;
                }
                
                dMin = rbClampF(dMin * 0.25f + 0.5f, 0.0f, 1.0f);
                dMax = rbClampF(dMax * 0.25f + 0.5f, 0.0f, 1.0f);
                yMin = height - (int32_t)roundf(dMax * (height - 1)) - 1;
                yMax = height - (int32_t)roundf(dMin * (height - 1));
                for(int32_t y = yMin; y < yMax; ++y) {
                    t2sett(pFrame, x, y, cadd(t2gett(pFrame, x, y), c));
                }
            }
        }
    }
}

