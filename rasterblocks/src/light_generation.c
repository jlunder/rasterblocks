#include "light_generation.h"

#include "graphics_util.h"


static RBPiecewiseLinearColorSegment g_rbWarmPalette[] = {
    {{  0,   0,   0, 255}, 2},
    {{ 63,   0,   0, 255}, 2},
    {{127,  15,   0, 255}, 2},
    {{255,  63,   0, 255}, 1},
    {{255, 127,   0, 255}, 1},
    {{255, 255,   0, 255}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbWarmPaletteAlpha[] = {
    {{  0,   0,   0,   0}, 2},
    {{ 63,   0,   0,  57}, 2},
    {{127,  15,   0, 113}, 2},
    {{255,  63,   0, 170}, 1},
    {{255, 127,   0, 198}, 1},
    {{255, 255,   0, 227}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbColdPalette[] = {
    {{  0,   0,   0, 255}, 1},
    {{  0,  63, 255, 255}, 1},
    {{127,   0,  63, 255}, 1},
    {{  0, 255,  63, 255}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbColdPaletteAlpha[] = {
    {{  0,   0,   0,   0}, 1},
    {{  0,  63, 255,  64}, 1},
    {{127,   0,  63, 128}, 1},
    {{  0, 255,  63, 192}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbGrayscalePalette[] = {
    {{  0,   0,   0, 255}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbGrayscalePaletteAlpha[] = {
    {{  0,   0,   0,   0}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbRainbowPalette[] = {
    {{255,   0,   0, 255}, 1},
    {{  0, 255,   0, 255}, 1},
    {{  0,   0, 255, 255}, 1},
    {{255,   0,   0, 255}, 0},
};

static uint8_t g_rbIconsData[][8] = {
#include "icons.h"
};

static char const * g_rbAmericanFlagData =
    "wbbbwbbbwbbbrrrrrrrrrrrrrrrr"
    "bbwbbbwbbbwbwwwwwwwwwwwwwwww"
    "wbbbwbbbwbbbrrrrrrrrrrrrrrrr"
    "bbwbbbwbbbwbwwwwwwwwwwwwwwww"
    "wbbbwbbbwbbbrrrrrrrrrrrrrrrr"
    "bbwbbbwbbbwbwwwwwwwwwwwwwwww"
    "wbbbwbbbwbbbrrrrrrrrrrrrrrrr"
    "wwwwwwwwwwwwwwwwwwwwwwwwwwww"
    "rrrrrrrrrrrrrrrrrrrrrrrrrrrr"
    "wwwwwwwwwwwwwwwwwwwwwwwwwwww"
    "rrrrrrrrrrrrrrrrrrrrrrrrrrrr"
    "wwwwwwwwwwwwwwwwwwwwwwwwwwww"
    "rrrrrrrrrrrrrrrrrrrrrrrrrrrr"
    ;

static char const * g_rbSeqCircLogoData =
    "                                                                                      "
    "                                                                                      "
    "                                                   XXXX                               "
    "                 XXXXXXXXXXXXXX                  XXXXXXXXX                            "
    "              XXXXXXXXXXXXXXXXXXXXXX           XXXXXXXXXXXXX                          "
    "           XXXXXXXXXXXXXXXXXXXXXXXXXXX       XXXXXXXXXXXXXXXX                         "
    "         XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXX                        "
    "        XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXXXXXX                       "
    "      XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX                      "
    "     XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX                      "
    "    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX                     "
    "   XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX                     "
    "   XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        XXXXXXXXXX  "
    "  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        XXXXXXXXXXX "
    "  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        XXXXXXXXXX "
    " XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  XXXXXXXXXXXXX        XXXXXXXXXX "
    " XXXXXXXXXXXXXXXXXX            XXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXX       XXXXXXXXXX "
    " XXXXXXXXXXXXXXX                  XXXXXXXXXXXXXXXXX     XXXXXXXXXXXX       XXXXXXXXXX "
    " XXXXXXXXXXXXXX                    XXXXXXXXXXXXXXX       XXXXXXXXXXX       XXXXXXXXXX "
    " XXXXXXXXXXXXX                      XXXXXXXXXXXXXX       XXXXXXXXXXXX      XXXXXXXXXX "
    "XXXXXXXXXXXXX                        XXXXXXXXXXXXX       XXXXXXXXXXXX      XXXXXXXXXXX"
    "XXXXXXXXXXXX                         XXXXXXXXXXXXX        XXXXXXXXXXX      XXXXXXXXXXX"
    "XXXXXXXXXXXX                          XXXXXXXXXXXX        XXXXXXXXXXX      XXXXXXXXXXX"
    "XXXXXXXXXXX                           XXXXXXXXXXXX        XXXXXXXXXXXX     XXXXXXXXXXX"
    "XXXXXXXXXXX                           XXXXXXXXXXXX         XXXXXXXXXXX     XXXXXXXXXXX"
    "XXXXXXXXXXX                           XXXXXXXXXXXXX        XXXXXXXXXXXX   XXXXXXXXXXXX"
    "XXXXXXXXXXX                            XXXXXXXXXXXX         XXXXXXXXXXXXXXXXXXXXXXXXX "
    "XXXXXXXXXXX                            XXXXXXXXXXXX         XXXXXXXXXXXXXXXXXXXXXXXXX "
    "XXXXXXXXXXX                           XXXXXXXXXXXXX          XXXXXXXXXXXXXXXXXXXXXXXX "
    "XXXXXXXXXXX                           XXXXXXXXXXXXXX         XXXXXXXXXXXXXXXXXXXXXXXX "
    "XXXXXXXXXXX                           XXXXXXXXXXXXXX         XXXXXXXXXXXXXXXXXXXXXXX  "
    "XXXXXXXXXXX                           XXXXXXXXXXXXXX          XXXXXXXXXXXXXXXXXXXXXX  "
    " XXXXXXXXXX                           XXXXXXXXXXXXXXX         XXXXXXXXXXXXXXXXXXXXX   "
    " XXXXXXXXXX                           XXXXXXXXXXXXXXX          XXXXXXXXXXXXXXXXXXXX   "
    " XXXXXXXXXXX                         XXXXXXXXXXXXXXXXX         XXXXXXXXXXXXXXXXXXX    "
    " XXXXXXXXXXX                         XXXXXXXXXXXXXXXXX          XXXXXXXXXXXXXXXXX     "
    "  XXXXXXXXXX                         XXXXXXXXXXXXXXXXXX           XXXXXXXXXXXXXX      "
    "  XXXXXXXXXXX                        XXXXXXXXXXXXXXXXXX            XXXXXXXXXXXX       "
    "  XXXXXXXXXXX                       XXXXXXXXXXXXXXXXXXXX             XXXXXXXXX        "
    "   XXXXXXXXXXX                     XXXXXXXXXXXXXXXXXXXXXX              XXXXX          "
    "                                                                                      "
    "                                                                                      "
    "                                                                                      "
    ;

static RBTime g_rbIconDebounceTime;
static RBTime g_rbIconDisplayTime;

RBTexture1 * g_rbPWarmPalTex = NULL;
RBTexture1 * g_rbPWarmPalAlphaTex = NULL;
RBTexture1 * g_rbPColdPalTex = NULL;
RBTexture1 * g_rbPColdPalAlphaTex = NULL;
RBTexture1 * g_rbPGrayscalePalTex = NULL;
RBTexture1 * g_rbPGrayscalePalAlphaTex = NULL;

RBTexture1 * g_rbPRainbowPalTex = NULL;

RBTexture2 * g_rbPAmericanFlagTex = NULL;
RBTexture2 * g_rbPSeqCircLogoTex = NULL;
RBTexture2 * g_rbPSeqCircLogoTex16x8 = NULL;
RBTexture2 * g_rbPSeqCircLogoTex32x16 = NULL;

static RBLightGenerator * g_rbPCurrentGenerator = NULL;


void rbLightGenerationInitialize(RBConfiguration const * config)
{
    UNUSED(config);
    
    rbLightGenerationShutdown();
    
    g_rbIconDebounceTime = rbTimeFromMs(50);
    g_rbIconDisplayTime = rbTimeFromMs(500);
    
    g_rbPWarmPalTex = rbTexture1Alloc(256);
    rbTexture1FillFromPiecewiseLinear(g_rbPWarmPalTex, g_rbWarmPalette,
        LENGTHOF(g_rbWarmPalette), false);
    
    g_rbPWarmPalAlphaTex = rbTexture1Alloc(256);
    rbTexture1FillFromPiecewiseLinear(g_rbPWarmPalAlphaTex,
        g_rbWarmPaletteAlpha, LENGTHOF(g_rbWarmPaletteAlpha), false);
    
    g_rbPColdPalTex = rbTexture1Alloc(256);
    rbTexture1FillFromPiecewiseLinear(g_rbPColdPalTex, g_rbColdPalette,
        LENGTHOF(g_rbColdPalette), false);
    
    g_rbPColdPalAlphaTex = rbTexture1Alloc(256);
    rbTexture1FillFromPiecewiseLinear(g_rbPColdPalAlphaTex,
        g_rbColdPaletteAlpha, LENGTHOF(g_rbColdPaletteAlpha), false);
    
    g_rbPGrayscalePalTex = rbTexture1Alloc(256);
    rbTexture1FillFromPiecewiseLinear(g_rbPGrayscalePalTex,
        g_rbGrayscalePalette, LENGTHOF(g_rbGrayscalePalette), false);
    
    g_rbPGrayscalePalAlphaTex = rbTexture1Alloc(256);
    rbTexture1FillFromPiecewiseLinear(g_rbPGrayscalePalAlphaTex,
        g_rbGrayscalePaletteAlpha, LENGTHOF(g_rbGrayscalePaletteAlpha), false);
    
    g_rbPRainbowPalTex = rbTexture1Alloc(3);
    rbTexture1FillFromPiecewiseLinear(g_rbPRainbowPalTex, g_rbRainbowPalette,
        LENGTHOF(g_rbRainbowPalette), true);
    
    UNUSED(g_rbIconsData);
    
    g_rbPAmericanFlagTex = rbTexture2Alloc(28, 13);
    for(size_t j = 0; j < t2geth(g_rbPAmericanFlagTex); ++j) {
        for(size_t i = 0; i < t2getw(g_rbPAmericanFlagTex); ++i) {
            RBColor c;
            switch(g_rbAmericanFlagData[j * t2getw(g_rbPAmericanFlagTex) + i]) {
            case 'r': c = colori(255, 0, 0, 255); break;
            case 'w': c = colori(255, 255, 255, 255); break;
            case 'b': c = colori(0, 0, 255, 255); break;
            default: c = colori(0, 0, 0, 255); break;
            }
            t2sett(g_rbPAmericanFlagTex, i, j, c);
        }
    }
    
    g_rbPSeqCircLogoTex = rbTexture2Alloc(86, 43);
    for(size_t j = 0; j < t2geth(g_rbPSeqCircLogoTex); ++j) {
        for(size_t i = 0; i < t2getw(g_rbPSeqCircLogoTex); ++i) {
            RBColor c;
            switch(g_rbSeqCircLogoData[j * t2getw(g_rbPSeqCircLogoTex) + i]) {
            case ' ': c = colori(0, 0, 0, 0); break;
            default: c = colori(255, 255, 255, 255); break;
            }
            t2sett(g_rbPSeqCircLogoTex, i, j, c);
        }
    }
    
    g_rbPSeqCircLogoTex16x8 = rbTexture2Alloc(16, 8);
    rbTexture2Rescale(g_rbPSeqCircLogoTex16x8, g_rbPSeqCircLogoTex);
    g_rbPSeqCircLogoTex32x16 = rbTexture2Alloc(32, 16);
    rbTexture2Rescale(g_rbPSeqCircLogoTex32x16, g_rbPSeqCircLogoTex);
    
    rbLightGenerationSetGenerator(
        rbLightGenerationStaticImageAlloc(g_rbPSeqCircLogoTex32x16));
}


void rbLightGenerationShutdown(void)
{
    rbLightGenerationSetGenerator(NULL);
    
    if(g_rbPWarmPalTex != NULL) {
        rbTexture1Free(g_rbPWarmPalTex);
        g_rbPWarmPalTex = NULL;
    }
    if(g_rbPWarmPalAlphaTex != NULL) {
        rbTexture1Free(g_rbPWarmPalAlphaTex);
        g_rbPWarmPalAlphaTex = NULL;
    }
    if(g_rbPColdPalTex != NULL) {
        rbTexture1Free(g_rbPColdPalTex);
        g_rbPColdPalTex = NULL;
    }
    if(g_rbPColdPalAlphaTex != NULL) {
        rbTexture1Free(g_rbPColdPalAlphaTex);
        g_rbPColdPalAlphaTex = NULL;
    }
    if(g_rbPGrayscalePalTex != NULL) {
        rbTexture1Free(g_rbPGrayscalePalTex);
        g_rbPGrayscalePalTex = NULL;
    }
    if(g_rbPGrayscalePalAlphaTex != NULL) {
        rbTexture1Free(g_rbPGrayscalePalAlphaTex);
        g_rbPGrayscalePalAlphaTex = NULL;
    }
    if(g_rbPRainbowPalTex != NULL) {
        rbTexture1Free(g_rbPRainbowPalTex);
        g_rbPRainbowPalTex = NULL;
    }
    if(g_rbPAmericanFlagTex != NULL) {
        rbTexture2Free(g_rbPAmericanFlagTex);
        g_rbPAmericanFlagTex = NULL;
    }
    if(g_rbPSeqCircLogoTex != NULL) {
        rbTexture2Free(g_rbPSeqCircLogoTex);
        g_rbPSeqCircLogoTex = NULL;
    }
    if(g_rbPSeqCircLogoTex16x8 != NULL) {
        rbTexture2Free(g_rbPSeqCircLogoTex16x8);
        g_rbPSeqCircLogoTex16x8 = NULL;
    }
    if(g_rbPSeqCircLogoTex32x16 != NULL) {
        rbTexture2Free(g_rbPSeqCircLogoTex32x16);
        g_rbPSeqCircLogoTex32x16 = NULL;
    }
}


void rbLightGenerationGenerate(RBAnalyzedAudio const * pAnalysis,
    RBTexture2 * pFrame)
{
    if(g_rbPCurrentGenerator != NULL) {
        rbLightGenerationGeneratorGenerate(g_rbPCurrentGenerator, pAnalysis,
            pFrame);
    }
    else {
        for(size_t j = 0; j < t2geth(pFrame); ++j) {
            for(size_t i = 0; i < t2getw(pFrame); ++i) {
                t2sett(pFrame, i, j, colori(255, 0, 255, 255));
            }
        }
    }
}


void rbLightGenerationSetGenerator(RBLightGenerator * pGenerator)
{
    if(g_rbPCurrentGenerator != NULL) {
        rbLightGenerationGeneratorFree(g_rbPCurrentGenerator);
    }
    g_rbPCurrentGenerator = pGenerator;
}


///////////////////////////////////////////////////////////////////////////////
// Compositor


typedef struct
{
    RBLightGenerator genDef;
    RBLightGenerator * pGenerators[4];
} RBLightGeneratorCompositor;


void rbLightGenerationCompositorFree(void * pData);
void rbLightGenerationCompositorGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationCompositor2Alloc(
    RBLightGenerator * pGenerator0, RBLightGenerator * pGenerator1)
{
    RBLightGeneratorCompositor * pCompositor =
        (RBLightGeneratorCompositor *)malloc(
            sizeof (RBLightGeneratorCompositor));
    
    pCompositor->genDef.pData = pCompositor;
    pCompositor->genDef.free = rbLightGenerationCompositorFree;
    pCompositor->genDef.generate = rbLightGenerationCompositorGenerate;
    
    rbZero(pCompositor->pGenerators, sizeof pCompositor->pGenerators);
    
    pCompositor->pGenerators[0] = pGenerator0;
    pCompositor->pGenerators[1] = pGenerator1;
    
    return &pCompositor->genDef;
}


void rbLightGenerationCompositorFree(void * pData)
{
    RBLightGeneratorCompositor * pCompositor =
        (RBLightGeneratorCompositor *)pData;
    
    for(size_t i = 0; i < LENGTHOF(pCompositor->pGenerators); ++i) {
        if(pCompositor->pGenerators[i] != NULL) {
            rbLightGenerationGeneratorFree(pCompositor->pGenerators[i]);
        }
    }
    free(pData);
}


void rbLightGenerationCompositorGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorCompositor * pCompositor =
        (RBLightGeneratorCompositor *)pData;
    
    // TODO
    UNUSED(pCompositor);
    UNUSED(pAnalysis);
    UNUSED(pFrame);
}


///////////////////////////////////////////////////////////////////////////////
// StaticImage


typedef struct
{
    RBLightGenerator genDef;
    RBTexture2 const * pTexture;
} RBLightGeneratorStaticImage;


void rbLightGenerationStaticImageGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationStaticImageAlloc(RBTexture2 const * pTex)
{
    RBLightGeneratorStaticImage * pStaticImage =
        (RBLightGeneratorStaticImage *)malloc(
            sizeof (RBLightGeneratorStaticImage));
    
    pStaticImage->genDef.pData = pStaticImage;
    pStaticImage->genDef.free = free;
    pStaticImage->genDef.generate = rbLightGenerationStaticImageGenerate;
    
    pStaticImage->pTexture = pTex;
    
    return &pStaticImage->genDef;
}


void rbLightGenerationStaticImageGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorStaticImage * pStaticImage =
        (RBLightGeneratorStaticImage *)pData;
    RBTexture2 const * pTex = pStaticImage->pTexture;
    
    UNUSED(pAnalysis);
    rbTexture2Blt(pFrame,
        ((int32_t)t2getw(pFrame) - (int32_t)t2getw(pTex)) / 2,
        ((int32_t)t2geth(pFrame) - (int32_t)t2geth(pTex)) / 2,
        t2getw(pTex), t2geth(pTex), pTex, 0, 0);
}


///////////////////////////////////////////////////////////////////////////////
// ImageFilter


typedef struct
{
    RBLightGenerator genDef;
    RBLightGenerator * pGenerator;
    RBTexture2 const * pTexture;
} RBLightGeneratorImageFilter;


void rbLightGenerationImageFilterFree(void * pData);
void rbLightGenerationImageFilterGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationImageFilterAlloc(
    RBLightGenerator * pGenerator, RBTexture2 const * pTex)
{
    RBLightGeneratorImageFilter * pImageFilter =
        (RBLightGeneratorImageFilter *)malloc(
            sizeof (RBLightGeneratorImageFilter));
    
    pImageFilter->genDef.pData = pImageFilter;
    pImageFilter->genDef.free = rbLightGenerationImageFilterFree;
    pImageFilter->genDef.generate = rbLightGenerationImageFilterGenerate;
    
    pImageFilter->pGenerator = pGenerator;
    pImageFilter->pTexture = pTex;
    
    return &pImageFilter->genDef;
}


void rbLightGenerationImageFilterFree(void * pData)
{
    RBLightGeneratorImageFilter * pImageFilter =
        (RBLightGeneratorImageFilter *)pData;
    
    rbLightGenerationGeneratorFree(pImageFilter->pGenerator);
    free(pImageFilter);
}


void rbLightGenerationImageFilterGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorImageFilter * pImageFilter =
        (RBLightGeneratorImageFilter *)pData;
    size_t const fWidth = t2getw(pFrame);
    size_t const fHeight = t2geth(pFrame);
    float const fwMul = 1.0f / fWidth;
    float const fhMul = 1.0f / fHeight;
    
    rbLightGenerationGeneratorGenerate(pImageFilter->pGenerator, pAnalysis,
        pFrame);
    for(size_t j = 0; j < fHeight; ++j) {
        for(size_t i = 0; i < fWidth; ++i) {
            t2sett(pFrame, i, j, cscalef(t2gett(pFrame, i, j),
                ctgeta(t2samplc(pImageFilter->pTexture,
                    vector2(i * fwMul, j * fhMul)))));
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// Rescale


typedef struct
{
    RBLightGenerator genDef;
    RBLightGenerator * pGenerator;
    RBTexture2 * pTexture;
} RBLightGeneratorRescale;


void rbLightGenerationRescaleFree(void * pData);
void rbLightGenerationRescaleGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationRescaleAlloc(
    RBLightGenerator * pGenerator, size_t srcWidth, size_t srcHeight)
{
    RBLightGeneratorRescale * pRescale =
        (RBLightGeneratorRescale *)malloc(
            sizeof (RBLightGeneratorRescale));
    
    pRescale->genDef.pData = pRescale;
    pRescale->genDef.free = rbLightGenerationRescaleFree;
    pRescale->genDef.generate = rbLightGenerationRescaleGenerate;
    
    pRescale->pGenerator = pGenerator;
    pRescale->pTexture = rbTexture2Alloc(srcWidth, srcHeight);
    
    return &pRescale->genDef;
}


void rbLightGenerationRescaleFree(void * pData)
{
    RBLightGeneratorRescale * pRescale = (RBLightGeneratorRescale *)pData;
    
    rbLightGenerationGeneratorFree(pRescale->pGenerator);
    rbTexture2Free(pRescale->pTexture);
    free(pRescale);
}


void rbLightGenerationRescaleGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorRescale * pRescale = (RBLightGeneratorRescale *)pData;
    
    rbLightGenerationGeneratorGenerate(pRescale->pGenerator, pAnalysis,
        pRescale->pTexture);
    rbTexture2Rescale(pFrame, pRescale->pTexture);
}


///////////////////////////////////////////////////////////////////////////////
// TimedRotation


void rbLightGenerationTimedRotationFree(void * pData);
void rbLightGenerationTimedRotationGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


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


///////////////////////////////////////////////////////////////////////////////
// PulsePlasma


void rbLightGenerationPulsePlasmaGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationPulsePlasmaAlloc(RBTexture1 * pPalTex);


///////////////////////////////////////////////////////////////////////////////
// GridPulse


void rbLightGenerationGridPulseGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationGridPulseAlloc(RBColor hColor,
    RBColor vColor);


///////////////////////////////////////////////////////////////////////////////
// DashedCircles


void rbLightGenerationDashedCirclesGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationDashedCirclesAlloc(RBTexture1 * pPalTex);


///////////////////////////////////////////////////////////////////////////////
// Fireworks


void rbLightGenerationFireworksGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationFireworksAlloc(RBTexture1 * pPalTex);


void rbLightGenerationAmericanFlagGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


///////////////////////////////////////////////////////////////////////////////
// AmericanFlag


RBLightGenerator * rbLightGenerationAmericanFlagAlloc(void)
{
    return rbLightGenerationStaticImageAlloc(g_rbPAmericanFlagTex);
}


///////////////////////////////////////////////////////////////////////////////
// IconCheckerboard


void rbLightGenerationIconCheckerboardGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationIconCheckerboardAlloc(void);


///////////////////////////////////////////////////////////////////////////////
// PulseCheckerboard


void rbLightGenerationPulseCheckerboardGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationPulseCheckerboardAlloc(void);


///////////////////////////////////////////////////////////////////////////////
// Lissajous


void rbLightGenerationLissajousGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationLissajousAlloc(void);


///////////////////////////////////////////////////////////////////////////////
// Oscilloscope


void rbLightGenerationOscilloscopeGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationOscilloscopeAlloc(void);


///////////////////////////////////////////////////////////////////////////////
// SeqCircLogo


RBLightGenerator * rbLightGenerationSeqCircLogoAlloc(void)
{
    return rbLightGenerationStaticImageAlloc(g_rbPSeqCircLogoTex32x16);
}


/*
static float g_rbFlagWave[RB_PROJECTION_WIDTH];


    float leftTreble = pAnalysis->trebleEnergy * sqrtf(1.0f - pAnalysis->leftRightBalance);
    float rightTreble = pAnalysis->trebleEnergy * sqrtf(pAnalysis->leftRightBalance);
    float bass = pAnalysis->bassEnergy;
    
    UNUSED(g_rbRainbowPalette);

//
    for(size_t i = 0; i < RB_PANEL_HEIGHT; ++i) {
        for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            int32_t k = i < 4 ? (int32_t)j + 3 - i: (int32_t)j - 4 + i;
            t2sett(pFrame, RB_PANEL_WIDTH - 1 - j, i,
                colorct(
                    t1sampnc(g_rbPColdTex,
                        leftTreble - k * (2.0f / RB_PANEL_WIDTH))));
        }
    }
    
    for(size_t i = 0; i < RB_PANEL_HEIGHT; ++i) {
        for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            int32_t k = i < 4 ? (int32_t)j + 3 - i: (int32_t)j - 4 + i;
            t2sett(pFrame, j + RB_PANEL_WIDTH, i,
                colorct(
                    t1sampnc(g_rbPColdTex,
                        rightTreble - k * (2.0f / RB_PANEL_WIDTH))));
        }
    }
//
    
    UNUSED(leftTreble);
    UNUSED(rightTreble);
    UNUSED(bass);
    
    //t2bltsa(pFrame, 0, 0, 32, 16, g_rbFlagTex, 0, 0);
    
    for(size_t i = 1; i < RB_PROJECTION_WIDTH; ++i) {
        g_rbFlagWave[RB_PROJECTION_WIDTH - i] =
            g_rbFlagWave[RB_PROJECTION_WIDTH - i - 1];
    }
    g_rbFlagWave[0] = bass;
    
    for(size_t j = 0; j < RB_PROJECTION_HEIGHT; ++j) {
        for(size_t i = 0; i < RB_PROJECTION_WIDTH; ++i) {
            t2sett(pFrame, i, j,
                colorct(ctscale(
                    t2sampnc(g_rbFlagTex,
                        vector2((i + 0.5f) / RB_PROJECTION_WIDTH,
                            (j + 0.5f) / RB_PROJECTION_HEIGHT)),
                    g_rbFlagWave[i])));
        }
    }
    
    //rbLightGenerationCompositeIcon(&lights->overhead, 2, 15);
    UNUSED(g_rbIcons);
*/
