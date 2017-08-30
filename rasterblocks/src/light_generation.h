#ifndef LIGHT_GENERATION_H_INCLUDED
#define LIGHT_GENERATION_H_INCLUDED


#include "rasterblocks.h"

#include "graphics_util.h"


typedef enum
{
    RBLGBM_SRC_ALPHA,
    RBLGBM_DEST_ALPHA,
    RBLGBM_STENCIL,
    RBLGBM_INV_STENCIL,
    RBLGBM_ADD,
    RBLGBM_SUBTRACT,
    RBLGBM_MULTIPLY,
} RBLightGenerationBlendMode;


typedef struct {
    void (*free)(void * pData);
    void (*generate)(void * pData, RBAnalyzedAudio const * pAnalysis,
        RBParameters const * pParameters, RBTexture2 * pFrame);
    void * pData;
} RBLightGenerator;


// Light generation subsystem
void rbLightGenerationInitialize(RBConfiguration const * pConfig);
void rbLightGenerationShutdown(void);
void rbLightGenerationGenerate(RBAnalyzedAudio const * pAnalysis,
    RBParameters const * pParameters, RBTexture2 * pFrame);

static inline void rbLightGenerationGeneratorFree(
    RBLightGenerator * pGenerator)
{
    pGenerator->free(pGenerator->pData);
}

static inline void rbLightGenerationGeneratorGenerate(
    RBLightGenerator * pGenerator, RBAnalyzedAudio const * pAnalysis,
    RBParameters const * pParameters, RBTexture2 * pFrame)
{
    pGenerator->generate(pGenerator->pData, pAnalysis, pParameters, pFrame);
}

void rbLightGenerationSetGenerator(RBLightGenerator * pGenerator);

extern RBLightGenerator * rbLightGenerationCompositorAlloc(void);
extern void rbLightGenerationCompositorAddLayer(
    RBLightGenerator * pCompositorGen, RBLightGenerator * pGenerator,
    RBTexture2 * pDestTexture, RBLightGenerationBlendMode blendMode,
    size_t alphaIndex, size_t transformPosIndex, size_t transformScaleIndex);

RBLightGenerator * rbLightGenerationSelectorAlloc(
    RBLightGenerator * const * pGenerators, size_t numGenerators,
    RBTime transitionTime, size_t selectIndex);

extern RBLightGenerator * rbLightGenerationDashedCirclesAlloc(
    RBTexture1 * pPalette, size_t scaleIndex, size_t dashScaleIndex,
    size_t rotationIndex);
extern RBLightGenerator * rbLightGenerationFillAlloc(size_t colorIndex);
extern RBLightGenerator * rbLightGenerationOscilloscopeAlloc(
    RBTexture1 * pPalette);
extern RBLightGenerator * rbLightGenerationPlasmaAlloc(RBTexture1 * pPalette,
    size_t scaleIndex);
extern RBLightGenerator * rbLightGenerationSignalLissajousAlloc(
    RBTexture1 * pPalette);
extern RBLightGenerator * rbLightGenerationStaticImageAlloc(
    RBTexture2 const * pTex);
extern RBLightGenerator * rbLightGenerationVerticalBarsAlloc(
    RBTexture1 * pPalette, size_t numBars, RBTime spawnInterval,
    RBTime fadeTime, size_t intensityIndex);


#endif

