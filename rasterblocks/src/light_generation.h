#ifndef LIGHT_GENERATION_H_INCLUDED
#define LIGHT_GENERATION_H_INCLUDED


#include "rasterblocks.h"

#include "graphics_util.h"


typedef struct {
    void (*free)(void * pData);
    void (*generate)(void * pData, RBAnalyzedAudio const * pAnalysis,
        RBTexture2 * pFrame);
    void * pData;
} RBLightGenerator;


// These palette textures fade from black through white; *Alpha* versions also
// fade alpha from 0 through 1, the non-alpha ones have alpha 1 throughout
// They are all meant to be used with clamp sampling
extern RBTexture1 * g_rbPBlackRedGoldWhiteFSPalTex;
extern RBTexture1 * g_rbPBlackRedGoldWhiteFSAlphaPalTex;
extern RBTexture1 * g_rbPBlackBlueGreenWhiteFSPalTex;
extern RBTexture1 * g_rbPBlackBlueGreenWhiteFSAlphaPalTex;
extern RBTexture1 * g_rbPBlackWhiteFSPalTex;
extern RBTexture1 * g_rbPBlackWhiteFSAlphaPalTex;
// This palette is meant to be used with repeat sampling
extern RBTexture1 * g_rbPRainbowFSPalTex;
extern RBTexture1 * g_rbPBlackGoldFSAlphaPalTex;
extern RBTexture1 * g_rbPBlackPaleBlueFSAlphaPalTex;
extern RBTexture1 * g_rbPBlackPaleGreenFSAlphaPalTex;
extern RBTexture1 * g_rbPBlackPurpleFSAlphaPalTex;
extern RBTexture1 * g_rbPGreenLavenderHSPalTex;
extern RBTexture1 * g_rbPBlackPurpleRedHSPalTex;
extern RBTexture1 * g_rbPBluePurpleGreenHSPalTex;
extern RBTexture1 * g_rbPBlueGoldHSPalTex;
extern RBTexture1 * g_rbPRedPinkHSPalTex;
extern RBTexture1 * g_rbPBlackRedGoldHSPalTex;
extern RBTexture1 * g_rbPBlackWhiteHSPalTex;


extern RBTexture2 * g_rbPSfuCsSurreyTex;


// Light generation subsystem
void rbLightGenerationInitialize(RBConfiguration const * pConfig);
void rbLightGenerationShutdown(void);
void rbLightGenerationGenerate(RBAnalyzedAudio const * pAnalysis,
    RBTexture2 * pFrame);

static inline void rbLightGenerationGeneratorFree(
    RBLightGenerator * pGenerator)
{
    pGenerator->free(pGenerator->pData);
}

static inline void rbLightGenerationGeneratorGenerate(
    RBLightGenerator * pGenerator, RBAnalyzedAudio const * pAnalysis,
    RBTexture2 * pFrame)
{
    pGenerator->generate(pGenerator->pData, pAnalysis, pFrame);
}

void rbLightGenerationSetGenerator(RBLightGenerator * pGenerator);

RBLightGenerator * rbLightGenerationCompositor2Alloc(
    RBLightGenerator * pGenerator0, RBLightGenerator * pGenerator1);
RBLightGenerator * rbLightGenerationCompositor3Alloc(
    RBLightGenerator * pGenerator0, RBLightGenerator * pGenerator1,
    RBLightGenerator * pGenerator2);
RBLightGenerator * rbLightGenerationCompositor4Alloc(
    RBLightGenerator * pGenerator0, RBLightGenerator * pGenerator1,
    RBLightGenerator * pGenerator2, RBLightGenerator * pGenerator3);
RBLightGenerator * rbLightGenerationStaticImageAlloc(RBTexture2 const * pTex);
RBLightGenerator * rbLightGenerationImageFilterAlloc(
    RBLightGenerator * pGenerator, RBTexture2 const * pTex);
RBLightGenerator * rbLightGenerationRescaleAlloc(
    RBLightGenerator * pGenerator, size_t srcWidth, size_t srcHeight);

RBLightGenerator * rbLightGenerationTimedRotationAlloc(
    RBLightGenerator * pGenerators[], size_t numGenerators, RBTime interval,
    int32_t controllerNum);
RBLightGenerator * rbLightGenerationControllerSelectAlloc(
    RBLightGenerator * pGenerators[], size_t numGenerators,
    int32_t controllerNum);
RBLightGenerator * rbLightGenerationControllerFadeAlloc(
    RBLightGenerator * pGenerator, int32_t controllerNum);
/*
void rbLightGenerationTimedRotationPauseRotation(
    RBLightGenerator * pTRGenerator);
void rbLightGenerationTimedRotationContinueRotation(
    RBLightGenerator * pTRGenerator);
void rbLightGenerationTimedRotationTransitionToGenerator(
    RBLightGenerator * pTRGenerator, RBLightGenerator * pGenerator);
void rbLightGenerationTimedRotationSetGenerator(
    RBLightGenerator * pTRGenerator, RBLightGenerator * pGenerator);
    */

RBLightGenerator * rbLightGenerationPlasmaAlloc(RBTexture1 * pPalTex);
RBLightGenerator * rbLightGenerationBeatFlashAlloc(RBTexture1 * pPalTex);
RBLightGenerator * rbLightGenerationPulsePlasmaAlloc(RBTexture1 * pPalTex);
RBLightGenerator * rbLightGenerationPulseGridAlloc(RBColor hColor,
    RBColor vColor);
RBLightGenerator * rbLightGenerationDashedCirclesAlloc(RBTexture1 * pPalTex);
RBLightGenerator * rbLightGenerationSmokeSignalsAlloc(RBTexture1 * pPalTex);
RBLightGenerator * rbLightGenerationFireworksAlloc(RBTexture1 * pPalTex);
RBLightGenerator * rbLightGenerationVerticalBarsAlloc(
    RBTexture1 * pBassPalTex, RBTexture1 * pTreblePalTex,
    size_t numBars, RBTime spawnInterval, RBTime fadeTime);
RBLightGenerator * rbLightGenerationCriscrossAlloc(RBTexture1 * pPalTex,
    size_t numBars, RBTime spawnInterval, RBTime fadeTime);
RBLightGenerator * rbLightGenerationVolumeBarsAlloc(RBTexture1 * pLowPalTex,
    RBTexture1 * pHiPalTex);
RBLightGenerator * rbLightGenerationBeatStarsAlloc(RBColor color);
RBLightGenerator * rbLightGenerationIconCheckerboardAlloc(RBColor color);
RBLightGenerator * rbLightGenerationPulseCheckerboardAlloc(RBColor color);
RBLightGenerator * rbLightGenerationParticleLissajousAlloc(RBColor color);
RBLightGenerator * rbLightGenerationSignalLissajousAlloc(RBColor color);
RBLightGenerator * rbLightGenerationOscilloscopeAlloc(RBColor color);


#endif

