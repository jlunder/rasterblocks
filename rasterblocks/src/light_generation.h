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

extern RBTexture2 * g_rbPAmericanFlagTex;
extern RBTexture2 * g_rbPSeqCircLogoTex;


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

RBLightGenerator * rbLightGenerationTimedRotationAlloc(void);
void rbLightGenerationTimedRotationAddGenerator(
    RBLightGenerator * pTRGenerator, RBLightGenerator * pGenerator);
void rbLightGenerationTimedRotationPauseRotation(
    RBLightGenerator * pTRGenerator);
void rbLightGenerationTimedRotationContinueRotation(
    RBLightGenerator * pTRGenerator);
void rbLightGenerationTimedRotationTransitionToGenerator(
    RBLightGenerator * pTRGenerator, RBLightGenerator * pGenerator);
void rbLightGenerationTimedRotationSetGenerator(
    RBLightGenerator * pTRGenerator, RBLightGenerator * pGenerator);

RBLightGenerator * rbLightGenerationPulsePlasmaAlloc(RBTexture1 * pPalTex);
RBLightGenerator * rbLightGenerationGridPulseAlloc(RBColor hColor,
    RBColor vColor);
RBLightGenerator * rbLightGenerationDashedCirclesAlloc(RBTexture1 * pPalTex);
RBLightGenerator * rbLightGenerationFireworksAlloc(RBTexture1 * pPalTex);
RBLightGenerator * rbLightGenerationAmericanFlagAlloc(void);
RBLightGenerator * rbLightGenerationIconCheckerboardAlloc(void);
RBLightGenerator * rbLightGenerationPulseCheckerboardAlloc(void);
RBLightGenerator * rbLightGenerationLissajousAlloc(void);
RBLightGenerator * rbLightGenerationOscilloscopeAlloc(void);
RBLightGenerator * rbLightGenerationSeqCircLogoAlloc(void);


#endif

