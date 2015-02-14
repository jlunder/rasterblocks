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
extern RBTexture1 * g_rbPWarmPalTex;
extern RBTexture1 * g_rbPWarmPalAlphaTex;
extern RBTexture1 * g_rbPColdPalTex;
extern RBTexture1 * g_rbPColdPalAlphaTex;
extern RBTexture1 * g_rbPGrayscalePalTex;
extern RBTexture1 * g_rbPGrayscalePalAlphaTex;

// This palette is meant to be used with repeat sampling
extern RBTexture1 * g_rbPRainbowPalTex;

extern RBTexture1 * g_rbPPal0Tex;
extern RBTexture1 * g_rbPPal1Tex;
extern RBTexture1 * g_rbPPal2Tex;
extern RBTexture1 * g_rbPPal3Tex;
extern RBTexture1 * g_rbPPal4Tex;
extern RBTexture1 * g_rbPPal5Tex;
extern RBTexture1 * g_rbPPal6Tex;
extern RBTexture1 * g_rbPPal7Tex;

extern RBTexture2 * g_rbPAmericanFlagTex;
extern RBTexture2 * g_rbPSeqCircLogoTex;
extern RBTexture2 * g_rbPSeqCircLogoTex16x8;
extern RBTexture2 * g_rbPSeqCircLogoTex24x12;
extern RBTexture2 * g_rbPSeqCircLogoTex32x16;
extern RBTexture2 * g_rbPVectorHarmonyLogoTex48x16;
extern RBTexture2 * g_rbPVectorHarmonyLogoTex24x8;


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
RBLightGenerator * rbLightGenerationStaticImageAlloc(RBTexture2 const * pTex);
RBLightGenerator * rbLightGenerationImageFilterAlloc(
    RBLightGenerator * pGenerator, RBTexture2 const * pTex);
RBLightGenerator * rbLightGenerationRescaleAlloc(
    RBLightGenerator * pGenerator, size_t srcWidth, size_t srcHeight);

RBLightGenerator * rbLightGenerationTimedRotationAlloc(
    RBLightGenerator * pGenerators[], size_t numGenerators);
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
RBLightGenerator * rbLightGenerationVolumeBarsAlloc(RBTexture1 * pLowPalTex,
    RBTexture1 * pHiPalTex);
RBLightGenerator * rbLightGenerationBeatStarsAlloc(RBColor color);
RBLightGenerator * rbLightGenerationIconCheckerboardAlloc(RBColor color);
RBLightGenerator * rbLightGenerationPulseCheckerboardAlloc(RBColor color);
RBLightGenerator * rbLightGenerationParticleLissajousAlloc(RBColor color);
RBLightGenerator * rbLightGenerationSignalLissajousAlloc(RBColor color);
RBLightGenerator * rbLightGenerationOscilloscopeAlloc(RBColor color);


#endif

