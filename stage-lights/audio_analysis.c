
#include <stdlib.h> 

#include "audio_analysis.h"
#include "fft.h"
#include "filters.h"

#include "analysis/levels.h"


#define SL_AGC_SAMPLE_WINDOW_S 4

static float const SL_AGC_ROOT_2 = 1.41421356237f;

#define NZEROS 4
#define NPOLES 4

// 100Hz cutoff
static float const g_slG100Hz = 5.543300177e+08;
static float const g_slK100Hz[NPOLES] = {
    -0.9663723877f,
    3.8985449174f,
    -5.8979669386f,
    3.9657943801f,
};

// 300Hz cutoff
static float const g_slG300Hz = 7.078510767e+06;
static float const g_slK300Hz[NPOLES] = {
    -0.9024653874f,
    3.7024671485f,
    -5.6973900039f,
    3.8973859825f,
};

static bool g_slAudioAnalysisFirstInit = false;

static float g_slXV100Hz[NZEROS + 1];
static float g_slYV100Hz[NPOLES + 1];

static float g_slXV300Hz[NZEROS + 1];
static float g_slYV300Hz[NPOLES + 1];

static float g_slAgcSamples[SL_VIDEO_FRAME_RATE * SL_AGC_SAMPLE_WINDOW_S];
static size_t g_slAgcIndex = 0;
static float g_slAgcTrackingValue = 1.0f;

static float g_slAgcMax = 1e-0f;
static float g_slAgcMin = 1e-2f;
static float g_slAgcStrength = 0.5f;


static void slAudioAnalysisLowPassFilter(float xv[NZEROS + 1],
    float yv[NPOLES + 1], float g, float const k[NPOLES],
    float const inputBuf[SL_AUDIO_FRAMES_PER_VIDEO_FRAME],
    float outputBuf[SL_AUDIO_FRAMES_PER_VIDEO_FRAME])
{
    for(size_t i = 0; i < SL_AUDIO_FRAMES_PER_VIDEO_FRAME; ++i) {
        xv[0] = xv[1]; xv[1] = xv[2]; xv[2] = xv[3]; xv[3] = xv[4]; 
        xv[4] = inputBuf[i] / g;
        yv[0] = yv[1]; yv[1] = yv[2]; yv[2] = yv[3]; yv[3] = yv[4]; 
        yv[4] =
            (xv[0] + xv[4]) +
            4 * (xv[1] + xv[3]) +
            6 * xv[2] +
            (k[0] * yv[0]) + (k[1] * yv[1]) +
            (k[2] * yv[2]) + (k[3] * yv[3]);
        outputBuf[i] = yv[4];
    }
}


void slAudioAnalysisInitialize(SLConfiguration const * config)
{
    UNUSED(config);
    if(!g_slAudioAnalysisFirstInit) {
        for(size_t i = 0; i < LENGTHOF(g_slAgcSamples); ++i) {
            g_slAgcSamples[i] = 1.0f;
        }
        g_slAudioAnalysisFirstInit = true;
    }
//    fftInitialize();
}


void slAudioAnalysisShutdown(void)
{
//	fftDestroy();
}


void slAudioAnalysisAnalyze(SLRawAudio const * audio, SLAnalyzedAudio * analysis)
{
    float inputBuf[SL_AUDIO_FRAMES_PER_VIDEO_FRAME];
    float buf100Hz[SL_AUDIO_FRAMES_PER_VIDEO_FRAME];
    float buf300Hz[SL_AUDIO_FRAMES_PER_VIDEO_FRAME];
    float bassPower = 0.0f;
    float midPower = 0.0f;
    float treblePower = 0.0f;
    
    float leftPower = 0.0f;
    float rightPower = 0.0f;
    
    float agcTarget = 0.0f;
    float agcValue = 1.0f;
    
    slAssert(SL_AUDIO_CHANNELS == 2);
    for(size_t i = 0; i < SL_AUDIO_FRAMES_PER_VIDEO_FRAME; ++i) {
        leftPower += audio->audio[i][0] * audio->audio[i][0] *
            (1.0f / SL_AUDIO_FRAMES_PER_VIDEO_FRAME);
        rightPower += audio->audio[i][1] * audio->audio[i][1] *
            (1.0f / SL_AUDIO_FRAMES_PER_VIDEO_FRAME);
        inputBuf[i] = (audio->audio[i][0] + audio->audio[i][1]) * 0.5;
    }
    leftPower = sqrtf(leftPower);
    rightPower = sqrtf(rightPower);
    
    slAudioAnalysisLowPassFilter(g_slXV100Hz, g_slYV100Hz, g_slG100Hz,
        g_slK100Hz, inputBuf, buf100Hz);
    slAudioAnalysisLowPassFilter(g_slXV300Hz, g_slYV300Hz, g_slG300Hz,
        g_slK300Hz, inputBuf, buf300Hz);
    
    for(size_t i = 0; i < SL_AUDIO_FRAMES_PER_VIDEO_FRAME; ++i) {
        bassPower += buf300Hz[i] * buf300Hz[i] *
            (1.0f / SL_AUDIO_FRAMES_PER_VIDEO_FRAME);
        midPower += (buf300Hz[i] - buf100Hz[i]) *
            (buf300Hz[i] - buf100Hz[i]) *
            (1.0f / SL_AUDIO_FRAMES_PER_VIDEO_FRAME);
        treblePower += (inputBuf[i] - buf300Hz[i]) *
            (inputBuf[i] - buf300Hz[i]) *
            (1.0f / SL_AUDIO_FRAMES_PER_VIDEO_FRAME);
    }
    
    analysis->bassEnergy = sqrtf(bassPower) * 2.0f;
    analysis->midEnergy = 0.0f;//1.0f + midPower * 20.0f;
    analysis->trebleEnergy = sqrtf(treblePower) * 2.0f;
    
    analysis->totalEnergy = (leftPower + rightPower) * 0.5f;
    if(analysis->totalEnergy > 0.0f) {
        analysis->leftRightBalance = rightPower / (leftPower + rightPower);
    }
    else {
        analysis->leftRightBalance = 0.5f;
    }
    
    g_slAgcIndex = (g_slAgcIndex + 1) % LENGTHOF(g_slAgcSamples);
    g_slAgcSamples[g_slAgcIndex] = analysis->totalEnergy * SL_AGC_ROOT_2;
    
    for(size_t i = 0; i < LENGTHOF(g_slAgcSamples); ++i) {
        agcTarget += g_slAgcSamples[i] * (1.0f / LENGTHOF(g_slAgcSamples));
        /*
        if(g_slAgcSamples[i] > agcTarget) {
            agcTarget = g_slAgcSamples[i];
        }
        */
    }
    g_slAgcTrackingValue = expf(
        (logf(g_slAgcMin) + logf(g_slAgcMax)) * 0.5f *
            (1.0f - g_slAgcStrength) +
        logf(agcTarget) * g_slAgcStrength);
    
    if(g_slAgcTrackingValue > g_slAgcMax) {
        g_slAgcTrackingValue = g_slAgcMax;
    }
    if(g_slAgcTrackingValue < g_slAgcMin) {
        g_slAgcTrackingValue = g_slAgcMin;
    }
    
    slInfo("AGC tracking %.4f from %.4f\n", g_slAgcTrackingValue, agcTarget);
    
    agcValue = 1.0f / g_slAgcTrackingValue;
    
    analysis->bassEnergy *= agcValue;
    analysis->midEnergy *= agcValue;
    analysis->trebleEnergy *= agcValue;
    
    analysis->totalEnergy *= agcValue;
}



