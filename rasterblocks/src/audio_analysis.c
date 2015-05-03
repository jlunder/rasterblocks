#include <stdlib.h>
#include <complex.h>
#include <math.h>

#include "audio_analysis.h"


#define RB_AGC_SAMPLE_WINDOW_S 4

#define NZEROS 4
#define NPOLES 4

// Filter state floating point type: internally we may need extra precision
typedef float RBFSFloat;
typedef complex double RBComplex;
#define rbSanitizeFSFloat rbSanitizeFloat

static RBFSFloat g_rbGLOW;
static RBFSFloat g_rbKLOW[NPOLES];

static RBFSFloat g_rbGHI;
static RBFSFloat g_rbKHI[NPOLES];

static bool g_rbAudioAnalysisFirstInit = false;

static RBFSFloat g_rbXVLOW[NZEROS + 1];
static RBFSFloat g_rbYVLOW[NPOLES + 1];

static RBFSFloat g_rbXVHI[NZEROS + 1];
static RBFSFloat g_rbYVHI[NPOLES + 1];

static float g_rbAgcSamples[RB_VIDEO_FRAME_RATE * RB_AGC_SAMPLE_WINDOW_S];
static size_t g_rbAgcIndex = 0;
static float g_rbAgcTrackingValue = 1.0f;

static float g_rbAgcMax;
static float g_rbAgcMin;
static float g_rbAgcStrength;

static RBTime g_rbPeakDebounceTime;
static float g_rbPeakDetectThreshold;
static float g_rbPeakResetThreshold;

static RBTimer g_rbPeakDebounceTimer;
static bool g_rbPeakDetected;


static void rbAudioAnalysisLowPassFilter(RBFSFloat xv[NZEROS + 1],
    RBFSFloat yv[NPOLES + 1], RBFSFloat g, RBFSFloat const k[NPOLES],
    float const inputBuf[RB_AUDIO_FRAMES_PER_VIDEO_FRAME],
    float outputBuf[RB_AUDIO_FRAMES_PER_VIDEO_FRAME])
{
    bool warningThrottle = false;
    
    // It's possible, for certain filter configurations, for xv or yv to
    // acquire NaN or +/-inf values. This is really problematic -- better that
    // the analysis be a little wrong than become infected with NaN! So
    // sanitize xv, yv before beginning work, and they won't persist for more
    // than one frame at least.
    for(size_t i = 0; i < NZEROS + 1; ++i) {
        rbSanitizeFSFloat(&xv[i], 0.0f);
    }
    for(size_t i = 0; i < NPOLES + 1; ++i) {
        rbSanitizeFSFloat(&xv[i], 0.0f);
    }
    
    for(size_t i = 0; i < RB_AUDIO_FRAMES_PER_VIDEO_FRAME; ++i) {
        RBFSFloat in = (RBFSFloat)inputBuf[i] / g;
        
        if(!warningThrottle) {
            if(isnan(in) || isinf(in)) {
                rbWarning("Bogus values in input!\n");
                warningThrottle = true;
            }
        }
        
        rbSanitizeFSFloat(&in, 0.0f);
        
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
    return RB_AUDIO_SAMPLE_RATE *
     tan(RB_PI_D * cutoffFreq / RB_AUDIO_SAMPLE_RATE) / RB_PI_D;
}

static void butterworthPoles(RBComplex* poles, double cutoff)
{
    double arg;
    for (int i = 0; i < NZEROS; i++) {
        arg = RB_PI_D * (2* i + NZEROS + 1)/2/NZEROS;
        poles[i] = cos(arg) + sin(arg) * I;
        poles[i] *= 2* RB_PI_D * cutoff;
    }
}

static void polesToZeros(RBComplex* poles, RBComplex* zeros)
{
    RBComplex defaultZero = (2*RB_AUDIO_SAMPLE_RATE + 0.0 * I);
    for (int i = 0; i < NZEROS; i++) {
        zeros[i] = (defaultZero + poles[i]) /  (defaultZero - poles[i]);
    }
}

static void zerosToCoeffs(RBComplex* zeros, RBComplex* coeffs)
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
        RBComplex sumVal = 1.0;
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

static void butterworthZZeros(RBComplex* zeros)
{
    for (int i = 0; i < NZEROS; i++) {
        zeros[i] = -1.0 + 0.0 * I;
    }
}

static RBFSFloat calculateGain(RBComplex *aCoeffs, RBComplex *bCoeffs)
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

static void rbAudioAnalysisCalculateCoefficients(double cutoffFreq,
    RBFSFloat* gainVal, RBFSFloat* coeffs)
{
    RBComplex pPoles[NPOLES];
    RBComplex zPoles[NPOLES];
    RBComplex bCoeffs[NPOLES+1];
    RBComplex zZeros[NPOLES];
    RBComplex aCoeffs[NPOLES+1];

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
        coeffs[i] = (RBFSFloat)creal(bCoeffs[i]);
    }
    *gainVal = calculateGain(aCoeffs, bCoeffs);
}

void rbAudioAnalysisInitialize(RBConfiguration const * config)
{
    rbAudioAnalysisCalculateCoefficients(config->lowCutoff, &g_rbGLOW, g_rbKLOW);
    rbAudioAnalysisCalculateCoefficients(config->hiCutoff, &g_rbGHI, g_rbKHI);
    g_rbAgcMax = config->agcMax;
    g_rbAgcMin = config->agcMin;
    g_rbAgcStrength = config->agcStrength;
    
    if(!rbIsRestarting()) {
        for(size_t i = 0; i < LENGTHOF(g_rbAgcSamples); ++i) {
            g_rbAgcSamples[i] = 1.0f;
        }
        g_rbAudioAnalysisFirstInit = true;
    }
    
    memset(g_rbXVLOW, 0, sizeof g_rbXVLOW);
    memset(g_rbYVLOW, 0, sizeof g_rbYVLOW);
    memset(g_rbXVHI, 0, sizeof g_rbXVHI);
    memset(g_rbYVHI, 0, sizeof g_rbYVHI);

    g_rbPeakDebounceTime = rbTimeFromMs(100);
    g_rbPeakDetectThreshold = 1.5f;
    g_rbPeakResetThreshold = 0.75f;
    
    rbStopTimer(&g_rbPeakDebounceTimer);
    g_rbPeakDetected = false;
}


void rbAudioAnalysisShutdown(void)
{
}

static void rbAudioAnalysisGatherChannels(RBRawAudio const * pAudio,
    float *leftPower, float *rightPower, float* inputBuf)
{
    for (size_t i = 0; i < RB_AUDIO_FRAMES_PER_VIDEO_FRAME; ++i) {
        *leftPower += pAudio->audio[i][0] * pAudio->audio[i][0] *
            (1.0f / RB_AUDIO_FRAMES_PER_VIDEO_FRAME);
        *rightPower += pAudio->audio[i][1] * pAudio->audio[i][1] *
            (1.0f / RB_AUDIO_FRAMES_PER_VIDEO_FRAME);
        inputBuf[i] = (pAudio->audio[i][0] + pAudio->audio[i][1]) * 0.5;
    }
    *leftPower = sqrtf(*leftPower);
    *rightPower = sqrtf(*rightPower);
}


static void rbAudioAnalysisCalculatePower(float * buf, float * pPower)
{
    float power = 0;
    
    for(size_t i = 0; i < RB_AUDIO_FRAMES_PER_VIDEO_FRAME; ++i) {
        power += buf[i] * buf[i];
    }
    
    *pPower = power / RB_AUDIO_FRAMES_PER_VIDEO_FRAME;
}


static void rbAudioAnalysisCalculateEnergy(RBAnalyzedAudio * analysis,
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


static void rbAudioAnalysisUpdateAgc(RBAnalyzedAudio * pAnalysis)
{
    float agcTarget = 0.0f;
    float agcValue = 1.0f;
    g_rbAgcIndex = (g_rbAgcIndex + 1) % LENGTHOF(g_rbAgcSamples);
    g_rbAgcSamples[g_rbAgcIndex] = pAnalysis->totalEnergy * RB_SQRT_2;
    
    if(g_rbAgcMin < 1.0e-10f || isinf(g_rbAgcMin) || isnan(g_rbAgcMin)) {
        rbWarning("Insane configured AGC min %g", g_rbAgcMin);
    }
    if(g_rbAgcMax < 1.0e-10f || isinf(g_rbAgcMax) || isnan(g_rbAgcMax)) {
        rbWarning("Insane configured AGC max %g", g_rbAgcMax);
    }
    
    for(size_t i = 0; i < LENGTHOF(g_rbAgcSamples); ++i) {
        rbSanitizeFloat(&g_rbAgcSamples[i], 1.0f);
        agcTarget += g_rbAgcSamples[i] * (1.0f / LENGTHOF(g_rbAgcSamples));
        /*
        if(g_rbAgcSamples[i] > agcTarget) {
            agcTarget = g_rbAgcSamples[i];
        }
        */
    }
    g_rbAgcTrackingValue = expf(
        (logf(g_rbAgcMin) + logf(g_rbAgcMax)) * 0.5f *
            (1.0f - g_rbAgcStrength) +
        logf(agcTarget) * g_rbAgcStrength);
    rbSanitizeFloat(&g_rbAgcTrackingValue, g_rbAgcMax);
    
    if(g_rbAgcTrackingValue > g_rbAgcMax) {
        g_rbAgcTrackingValue = g_rbAgcMax;
    }
    if(g_rbAgcTrackingValue < g_rbAgcMin) {
        g_rbAgcTrackingValue = g_rbAgcMin;
    }
    
    rbInfo("AGC tracking %.4f from %.4f\n", g_rbAgcTrackingValue, agcTarget);
    
    agcValue = 0.7f / g_rbAgcTrackingValue;
    
    for(size_t i = 0; i < RB_AUDIO_FRAMES_PER_VIDEO_FRAME; ++i) {
        pAnalysis->rawAudio[i] *= agcValue;
        pAnalysis->bassAudio[i] *= agcValue;
        pAnalysis->trebleAudio[i] *= agcValue;
    }
    
    pAnalysis->bassEnergy *= agcValue;
    pAnalysis->midEnergy *= agcValue;
    pAnalysis->trebleEnergy *= agcValue;
    
    pAnalysis->totalEnergy *= agcValue;
    
    pAnalysis->agcValue = g_rbAgcTrackingValue;
}


void rbAudioAnalysisAnalyze(RBRawAudio const * pAudio,
    RBAnalyzedAudio * pAnalysis)
{
    float inputBuf[RB_AUDIO_FRAMES_PER_VIDEO_FRAME];
    float bufLOW[RB_AUDIO_FRAMES_PER_VIDEO_FRAME];
    float bufMID[RB_AUDIO_FRAMES_PER_VIDEO_FRAME];
    float bufHI[RB_AUDIO_FRAMES_PER_VIDEO_FRAME];
    float bassPower = 0.0f;
    float midPower = 0.0f;
    float treblePower = 0.0f;
    
    float leftPower = 0.0f;
    float rightPower = 0.0f;
        
    rbAssert(RB_AUDIO_CHANNELS == 2);
    rbAudioAnalysisGatherChannels(pAudio, &leftPower, &rightPower, inputBuf);
    
    rbAudioAnalysisLowPassFilter(g_rbXVLOW, g_rbYVLOW, g_rbGLOW,
        g_rbKLOW, inputBuf, bufLOW);
    rbAudioAnalysisLowPassFilter(g_rbXVHI, g_rbYVHI, g_rbGHI,
        g_rbKHI, inputBuf, bufHI);

    for(size_t i = 0; i < RB_AUDIO_FRAMES_PER_VIDEO_FRAME; ++i) {
        bufMID[i] += bufHI[i] - bufLOW[i];
        bufHI[i] = inputBuf[i] - bufHI[i];
    }
    rbAudioAnalysisCalculatePower(bufLOW, &bassPower);
    rbAudioAnalysisCalculatePower(bufMID, &midPower);
    rbAudioAnalysisCalculatePower(bufHI, &treblePower);
    
    pAnalysis->frameNum = pAudio->frameNum;
    
    rbAudioAnalysisCalculateEnergy(pAnalysis, bassPower, midPower, treblePower,
        leftPower, rightPower);
    
    memcpy(pAnalysis->rawAudio, inputBuf, sizeof pAnalysis->rawAudio);
    memcpy(pAnalysis->bassAudio, bufLOW, sizeof pAnalysis->bassAudio);
    memcpy(pAnalysis->trebleAudio, bufHI, sizeof pAnalysis->trebleAudio);
    
    rbAudioAnalysisUpdateAgc(pAnalysis);
    
    pAnalysis->sourceOverdriven = pAudio->overdriven;
    pAnalysis->sourceLargeDc = pAudio->largeDc;
    
    pAnalysis->peakDetected = false;
    if(g_rbPeakDetected) {
        if(pAnalysis->bassEnergy < g_rbPeakResetThreshold &&
//                pAnalysis->trebleEnergy < g_rbPeakResetThreshold &&
                rbTimerElapsed(&g_rbPeakDebounceTimer)) {
            g_rbPeakDetected = false;
            rbStopTimer(&g_rbPeakDebounceTimer);
        }
    }
    else if(pAnalysis->bassEnergy > g_rbPeakDetectThreshold) {
//    else if(pAnalysis->bassEnergy > g_rbPeakDetectThreshold &&
//            pAnalysis->trebleEnergy > g_rbPeakDetectThreshold) {
        g_rbPeakDetected = true;
        rbStartTimer(&g_rbPeakDebounceTimer, g_rbPeakDebounceTime);
        pAnalysis->peakDetected = true;
    }
}



