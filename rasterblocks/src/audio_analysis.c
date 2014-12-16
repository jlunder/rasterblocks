
#include <stdlib.h>
#include <complex.h>
#include <math.h>

#include "audio_analysis.h"

#define SL_AGC_SAMPLE_WINDOW_S 4
#define M_PI 3.14159265358979323846

static float const SL_AGC_ROOT_2 = 1.41421356237f;

#define NZEROS 4
#define NPOLES 4

// Filter state floating point type: internally we may need extra precision
typedef float slFSFloat;
#define slSanitizeFSFloat slSanitizeFloat

// 200Hz cutoff (For now)
static slFSFloat g_slGLOW = 3.523725849e+07;
static slFSFloat g_slKLOW[NPOLES] = {
    -0.9338739884f,
    3.7993827672f,
    -5.7970987038f,
    3.9315894710f,
};

// 300Hz cutoff
static slFSFloat g_slGHI = 7.078510767e+06;
static slFSFloat g_slKHI[NPOLES] = {
    -0.9024653874f,
    3.7024671485f,
    -5.6973900039f,
    3.8973859825f,
};

static bool g_slAudioAnalysisFirstInit = false;

static slFSFloat g_slXVLOW[NZEROS + 1];
static slFSFloat g_slYVLOW[NPOLES + 1];

static slFSFloat g_slXVHI[NZEROS + 1];
static slFSFloat g_slYVHI[NPOLES + 1];

static float g_slAgcSamples[SL_VIDEO_FRAME_RATE * SL_AGC_SAMPLE_WINDOW_S];
static size_t g_slAgcIndex = 0;
static float g_slAgcTrackingValue = 1.0f;

static float g_slAgcMax = 1e-0f;
static float g_slAgcMin = 1e-2f;
static float g_slAgcStrength = 0.5f;


static void slAudioAnalysisLowPassFilter(slFSFloat xv[NZEROS + 1],
    slFSFloat yv[NPOLES + 1], slFSFloat g, slFSFloat const k[NPOLES],
    float const inputBuf[SL_AUDIO_FRAMES_PER_VIDEO_FRAME],
    float outputBuf[SL_AUDIO_FRAMES_PER_VIDEO_FRAME])
{
    bool warningThrottle = false;
    
    // It's possible, for certain filter configurations, for xv or yv to
    // acquire NaN or +/-inf values. This is really problematic -- better that
    // the analysis be a little wrong than become infected with NaN! So
    // sanitize xv, yv before beginning work, and they won't persist for more
    // than one frame at least.
    for(size_t i = 0; i < NZEROS + 1; ++i) {
        slSanitizeFSFloat(&xv[i], 0.0f);
    }
    for(size_t i = 0; i < NPOLES + 1; ++i) {
        slSanitizeFSFloat(&xv[i], 0.0f);
    }
    
    for(size_t i = 0; i < SL_AUDIO_FRAMES_PER_VIDEO_FRAME; ++i) {
        slFSFloat in = (slFSFloat)inputBuf[i] / g;
        
        if(!warningThrottle) {
            if(isnan(in) || isinf(in)) {
                slWarning("Bogus values in input!\n");
                warningThrottle = true;
            }
        }
        
        slSanitizeFSFloat(&in, 0.0f);
        
        xv[0] = xv[1]; xv[1] = xv[2]; xv[2] = xv[3]; xv[3] = xv[4]; 
        xv[4] = in;
        yv[0] = yv[1]; yv[1] = yv[2]; yv[2] = yv[3]; yv[3] = yv[4]; 
        yv[4] =
            (xv[0] + xv[4]) +
            4 * (xv[1] + xv[3]) +
            6 * xv[2] +
            (k[0] * yv[0]) + (k[1] * yv[1]) +
            (k[2] * yv[2]) + (k[3] * yv[3]);
        outputBuf[i] = (float)yv[4];
    }
}

static double warpFrequency(double cutoffFreq)
{
    return SL_AUDIO_SAMPLE_RATE *
     tan(M_PI * cutoffFreq / SL_AUDIO_SAMPLE_RATE) / M_PI;
}

static void butterworthPoles(complex* poles, double cutoff)
{
    double arg;
    for (int i = 0; i < NZEROS; i++) {
        arg = M_PI * (2* i + NZEROS + 1)/2/NZEROS;
        poles[i] = cos(arg) + sin(arg) * I;
        poles[i] *= 2* M_PI * cutoff;
    }
}

static void polesToZeros(complex* poles, complex* zeros)
{
    complex defaultZero = (2*SL_AUDIO_SAMPLE_RATE + 0.0 * I);
    for (int i = 0; i < NZEROS; i++) {
        zeros[i] = (defaultZero + poles[i]) /  (defaultZero - poles[i]);
    }
}

static void zerosToCoeffs(complex* zeros, complex* coeffs)
{
    int b[NPOLES+1];
    memset(b, 0, sizeof(int) * (NPOLES+1));
    int i = 0;
    while ( !b[NPOLES] ) {
        i = 0;
        while (b[i]) {
            b[i++] = 0;
        }
        b[i] = 1;

        int subsetSize = 0;
        complex sumVal = 1.0;
        for (i = 0; i < NPOLES; i++) {
            if(b[i]) {
                sumVal *= zeros[i];
                ++subsetSize;
            }
        }
        coeffs[NPOLES - subsetSize] += sumVal;
    }
    coeffs[NPOLES] = 1.0 + 0.0 * I;
}

static void butterworthZZeros(complex* zeros)
{
    for (int i = 0; i < NZEROS; i++) {
        zeros[i] = -1.0 + 0.0 * I;
    }
}

static slFSFloat calculateGain(complex *aCoeffs, complex *bCoeffs)
{
    double kNumer = 0.0f;
    double kDenom = 0.0f;
    for (int i = 0; i < NZEROS+1; i++) {
        kNumer += creal(aCoeffs[i]);
    }
    for (int i = 0; i < NPOLES; i++) {
        kDenom += creal(bCoeffs[i]);
    }
    return kNumer/(1.0f - kDenom);
}

static void slAudioAnalysisCalculateCoefficients(double cutoffFreq,
    slFSFloat* gainVal, slFSFloat* coeffs)
{
    complex pPoles[NPOLES];
    complex zPoles[NPOLES];
    complex bCoeffs[NPOLES+1];
    complex zZeros[NPOLES];
    complex aCoeffs[NPOLES+1];

    memset(pPoles, 0, sizeof pPoles);
    memset(zPoles, 0, sizeof zPoles);
    memset(bCoeffs, 0, sizeof bCoeffs);
    memset(zZeros, 0, sizeof zZeros);
    memset(aCoeffs, 0, sizeof aCoeffs);

    double warpedCutoff = warpFrequency(cutoffFreq);

    butterworthPoles(pPoles, warpedCutoff);
    polesToZeros(pPoles, zPoles);
    zerosToCoeffs(zPoles, bCoeffs);

    for (int i = (NPOLES)%2; i < NPOLES; i+= 2){
        bCoeffs[i] = -bCoeffs[i];
    }

    butterworthZZeros(zZeros);
    zerosToCoeffs(zZeros, aCoeffs);

    for (int i = (NPOLES+1)%2; i < NPOLES; i+= 2){
        aCoeffs[i] = -aCoeffs[i];
    }

    for(int i=0; i<NZEROS; ++i){
        coeffs[i] = (slFSFloat)creal(bCoeffs[i]);
    }
    *gainVal = calculateGain(aCoeffs, bCoeffs);
}

void slAudioAnalysisInitialize(SLConfiguration const * config)
{
    slAudioAnalysisCalculateCoefficients(config->lowCutoff, &g_slGLOW, g_slKLOW);
    slAudioAnalysisCalculateCoefficients(config->hiCutoff, &g_slGHI, g_slKHI);
    g_slAgcMax = config->agcMax;
    g_slAgcMin = config->agcMin;
    g_slAgcStrength = config->agcStrength;

    if(!g_slAudioAnalysisFirstInit) {
        for(size_t i = 0; i < LENGTHOF(g_slAgcSamples); ++i) {
            g_slAgcSamples[i] = 1.0f;
        }
        g_slAudioAnalysisFirstInit = true;
    }
    
    memset(g_slXVLOW, 0, sizeof g_slXVLOW);
    memset(g_slYVLOW, 0, sizeof g_slYVLOW);
    memset(g_slXVHI, 0, sizeof g_slXVHI);
    memset(g_slYVHI, 0, sizeof g_slYVHI);
}


void slAudioAnalysisShutdown(void)
{
}

static void slAudioAnalysisGatherChannels(SLRawAudio const * audio,
    float *leftPower, float *rightPower, float* inputBuf)
{
    for (size_t i = 0; i < SL_AUDIO_FRAMES_PER_VIDEO_FRAME; ++i) {
        *leftPower += audio->audio[i][0] * audio->audio[i][0] *
            (1.0f / SL_AUDIO_FRAMES_PER_VIDEO_FRAME);
        *rightPower += audio->audio[i][1] * audio->audio[i][1] *
            (1.0f / SL_AUDIO_FRAMES_PER_VIDEO_FRAME);
        inputBuf[i] = (audio->audio[i][0] + audio->audio[i][1]) * 0.5;
    }
    *leftPower = sqrtf(*leftPower);
    *rightPower = sqrtf(*rightPower);
}


static void slAudioAnalysisCalculatePower(
    float* inputBuf, float* bufLow, float* bufHi,
    float *bassPower, float *midPower, float *treblePower)
{
    for(size_t i = 0; i < SL_AUDIO_FRAMES_PER_VIDEO_FRAME; ++i) {
        *bassPower += bufLow[i] * bufLow[i] *
            (1.0f / SL_AUDIO_FRAMES_PER_VIDEO_FRAME);

        *midPower += (bufHi[i] - bufLow[i]) *
            (bufHi[i] - bufLow[i]) *
            (1.0f / SL_AUDIO_FRAMES_PER_VIDEO_FRAME);

        *treblePower += (inputBuf[i] - bufHi[i]) *
            (inputBuf[i] - bufHi[i]) *
            (1.0f / SL_AUDIO_FRAMES_PER_VIDEO_FRAME);
    }
}

static void slAudioAnalysisCalculateEnergy( SLAnalyzedAudio * analysis,
    float bassPower, float midPower, float treblePower,
    float leftPower, float rightPower)
{
    analysis->bassEnergy = sqrtf(bassPower) * 2.0f;
    analysis->midEnergy = sqrtf(midPower) * 2.0f;
    analysis->trebleEnergy = sqrtf(treblePower) * 2.0f;
    
    analysis->totalEnergy = (leftPower + rightPower) * 0.5f;
    if(analysis->totalEnergy > 0.0f) {
        analysis->leftRightBalance = rightPower / (leftPower + rightPower);
    }
    else {
        analysis->leftRightBalance = 0.5f;
    }
}

static void slAudioAnalysisUpdateAgc(SLAnalyzedAudio * analysis)
{
    float agcTarget = 0.0f;
    float agcValue = 1.0f;
    g_slAgcIndex = (g_slAgcIndex + 1) % LENGTHOF(g_slAgcSamples);
    g_slAgcSamples[g_slAgcIndex] = analysis->totalEnergy * SL_AGC_ROOT_2;
    
    for(size_t i = 0; i < LENGTHOF(g_slAgcSamples); ++i) {
        slSanitizeFloat(&g_slAgcSamples[i], 1.0f);
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
    slSanitizeFloat(&g_slAgcTrackingValue, g_slAgcMax);
    
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


void slAudioAnalysisAnalyze(SLRawAudio const * audio, SLAnalyzedAudio * analysis)
{
    float inputBuf[SL_AUDIO_FRAMES_PER_VIDEO_FRAME];
    float bufLOW[SL_AUDIO_FRAMES_PER_VIDEO_FRAME];
    float bufHI[SL_AUDIO_FRAMES_PER_VIDEO_FRAME];
    float bassPower = 0.0f;
    float midPower = 0.0f;
    float treblePower = 0.0f;
    
    float leftPower = 0.0f;
    float rightPower = 0.0f;
        
    slAssert(SL_AUDIO_CHANNELS == 2);
    slAudioAnalysisGatherChannels(audio, &leftPower, &rightPower, inputBuf);
    
    slAudioAnalysisLowPassFilter(g_slXVLOW, g_slYVLOW, g_slGLOW,
        g_slKLOW, inputBuf, bufLOW);
    slAudioAnalysisLowPassFilter(g_slXVHI, g_slYVHI, g_slGHI,
        g_slKHI, inputBuf, bufHI);

    slAudioAnalysisCalculatePower(inputBuf, bufLOW, bufHI,
        &bassPower, &midPower, &treblePower);

    slAudioAnalysisCalculateEnergy(analysis, bassPower, midPower, treblePower, leftPower, rightPower);

    slAudioAnalysisUpdateAgc(analysis);
}



