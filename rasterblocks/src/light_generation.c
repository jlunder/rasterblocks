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

RBTexture2 * g_rbPIconTexs[LENGTHOF(g_rbIconsData)];

static RBLightGenerator * g_rbPCurrentGenerator = NULL;


void rbLightGenerationInitialize(RBConfiguration const * config)
{
    UNUSED(config);
    
    rbLightGenerationShutdown();
    
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
    
    for(size_t k = 0; k < LENGTHOF(g_rbIconsData); ++k) {
        g_rbPIconTexs[k] = rbTexture2Alloc(8, 8);
        
        for(size_t j = 0; j < 8; ++j) {
            for(size_t i = 0; i < 8; ++i) {
                t2sett(g_rbPIconTexs[k], i, j,
                    ((g_rbIconsData[k][j] >> i) & 1) != 0 ?
                        colori(255, 255, 255, 255) : colori(0, 0, 0, 0));
            }
        }
    }
    
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
        rbLightGenerationCompositor2Alloc(
            rbLightGenerationPulseGridAlloc(colori(63, 0, 0, 255),
                colori(0, 0, 63, 255)),
            rbLightGenerationBeatFlashAlloc(g_rbPGrayscalePalAlphaTex)
        ));
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
    for(size_t k = 0; k < LENGTHOF(g_rbIconsData); ++k) {
        if(g_rbPIconTexs[k] != NULL) {
            rbTexture2Free(g_rbPIconTexs[k]);
            g_rbPIconTexs[k] = NULL;
        }
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
    size_t const fWidth = t2getw(pFrame);
    size_t const fHeight = t2geth(pFrame);
    RBTexture2 * pTempTex = rbTexture2Alloc(fWidth, fHeight);
    
    for(size_t i = 0; i < LENGTHOF(pCompositor->pGenerators); ++i) {
        if(i == 0) {
            rbLightGenerationGeneratorGenerate(pCompositor->pGenerators[i],
                pAnalysis, pFrame);
        }
        else if(pCompositor->pGenerators[i] != NULL) {
            rbLightGenerationGeneratorGenerate(pCompositor->pGenerators[i],
                pAnalysis, pTempTex);
            rbTexture2BltSrcAlpha(pFrame, 0, 0, fWidth, fHeight, pTempTex,
                0, 0);
        }
    }
    
    rbTexture2Free(pTempTex);
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
            t2sett(pFrame, i, j, colorct(ctmul(
                colortempc(t2gett(pFrame, i, j)),
                t2samplc(pImageFilter->pTexture,
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


typedef struct
{
    RBLightGenerator genDef;
    size_t numGenerators;
    size_t curGenerator;
    size_t lastGenerator;
    RBTimer rotationTimer;
    RBTimer transitionTimer;
    RBLightGenerator * pGenerators[64];
} RBLightGeneratorTimedRotation;


#define RB_ROTATION_TIME_MS 60000
#define RB_TRANSITION_TIME_MS 1000


void rbLightGenerationTimedRotationFree(void * pData);
void rbLightGenerationTimedRotationGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationTimedRotationAlloc(
    RBLightGenerator * pGenerators[], size_t numGenerators)
{
    RBLightGeneratorTimedRotation * pTimedRotation =
        (RBLightGeneratorTimedRotation *)malloc(
            sizeof (RBLightGeneratorTimedRotation));
    
    pTimedRotation->genDef.pData = pTimedRotation;
    pTimedRotation->genDef.free = rbLightGenerationTimedRotationFree;
    pTimedRotation->genDef.generate = rbLightGenerationTimedRotationGenerate;
    
    if(numGenerators > LENGTHOF(pTimedRotation->pGenerators)) {
        pTimedRotation->numGenerators = LENGTHOF(pTimedRotation->pGenerators);
    }
    else {
        pTimedRotation->numGenerators = numGenerators;
    }
    memcpy(pTimedRotation->pGenerators, pGenerators,
        pTimedRotation->numGenerators * sizeof (RBLightGenerator *));
    
    pTimedRotation->curGenerator = rand() % pTimedRotation->numGenerators;
    pTimedRotation->lastGenerator = 0;
    
    rbStartTimer(&pTimedRotation->rotationTimer,
        rbTimeFromMs(RB_ROTATION_TIME_MS));
    rbStopTimer(&pTimedRotation->transitionTimer);
    
    return &pTimedRotation->genDef;
}


void rbLightGenerationTimedRotationFree(void * pData)
{
    RBLightGeneratorTimedRotation * pTimedRotation =
        (RBLightGeneratorTimedRotation *)pData;
    
    for(size_t i = 0; i < pTimedRotation->numGenerators; ++i) {
        rbLightGenerationGeneratorFree(pTimedRotation->pGenerators[i]);
    }
    free(pTimedRotation);
}


void rbLightGenerationTimedRotationGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorTimedRotation * pTimedRotation =
        (RBLightGeneratorTimedRotation *)pData;
    
    if(rbGetTimerPeriodsAndReset(&pTimedRotation->rotationTimer) != 0) {
        pTimedRotation->lastGenerator = pTimedRotation->curGenerator;
        do {
            pTimedRotation->curGenerator =
                rand() % pTimedRotation->numGenerators;
        } while(pTimedRotation->numGenerators > 1 &&
            pTimedRotation->curGenerator == pTimedRotation->lastGenerator);
        rbStartTimer(&pTimedRotation->transitionTimer,
            rbTimeFromMs(RB_TRANSITION_TIME_MS));
    }
    
    if(rbTimerElapsed(&pTimedRotation->transitionTimer)) {
        rbLightGenerationGeneratorGenerate(
            pTimedRotation->pGenerators[pTimedRotation->curGenerator],
            pAnalysis, pFrame);
    }
    else {
        size_t const fWidth = t2getw(pFrame);
        size_t const fHeight = t2geth(pFrame);
        RBTexture2 * pTempTexA = rbTexture2Alloc(fWidth, fHeight);
        RBTexture2 * pTempTexB = rbTexture2Alloc(fWidth, fHeight);
        float alpha = (float)rbGetTimeLeft(&pTimedRotation->transitionTimer) /
            (float)rbTimeFromMs(RB_TRANSITION_TIME_MS);
        
        rbLightGenerationGeneratorGenerate(
            pTimedRotation->pGenerators[pTimedRotation->lastGenerator],
            pAnalysis, pTempTexA);
        rbLightGenerationGeneratorGenerate(
            pTimedRotation->pGenerators[pTimedRotation->curGenerator],
            pAnalysis, pTempTexB);
        
        rbTexture2Mix(pFrame, pTempTexA, 1.0f - alpha, pTempTexB, alpha);
        
        rbTexture2Free(pTempTexA);
        rbTexture2Free(pTempTexB);
    }
}


///////////////////////////////////////////////////////////////////////////////
// BeatFlash


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
    RBTimer flashTimer;
} RBLightGeneratorBeatFlash;


#define RB_BEAT_FLASH_TIME_MS 250


void rbLightGenerationBeatFlashFree(void * pData);
void rbLightGenerationBeatFlashGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationBeatFlashAlloc(RBTexture1 * pPalTex)
{
    RBLightGeneratorBeatFlash * pBeatFlash =
        (RBLightGeneratorBeatFlash *)malloc(
            sizeof (RBLightGeneratorBeatFlash));
    
    pBeatFlash->genDef.pData = pBeatFlash;
    pBeatFlash->genDef.free = rbLightGenerationBeatFlashFree;
    pBeatFlash->genDef.generate = rbLightGenerationBeatFlashGenerate;
    pBeatFlash->pPalTex = pPalTex;
    rbStopTimer(&pBeatFlash->flashTimer);
    
    return &pBeatFlash->genDef;
}


void rbLightGenerationBeatFlashFree(void * pData)
{
    RBLightGeneratorBeatFlash * pBeatFlash =
        (RBLightGeneratorBeatFlash *)pData;
    
    free(pBeatFlash);
}


void rbLightGenerationBeatFlashGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorBeatFlash * pBeatFlash =
        (RBLightGeneratorBeatFlash *)pData;
    RBTime const flashTime = rbTimeFromMs(RB_BEAT_FLASH_TIME_MS);
    RBColor fillColor;
    
    if(pAnalysis->peakDetected) {
        rbStartTimer(&pBeatFlash->flashTimer, flashTime);
    }
    
    if(!rbTimerElapsed(&pBeatFlash->flashTimer)) {
        float flashAlpha = (float)rbGetTimeLeft(&pBeatFlash->flashTimer) /
            (float)flashTime;
        fillColor = colorct(t1samplc(pBeatFlash->pPalTex,
            flashAlpha * flashAlpha * flashAlpha * flashAlpha));
    }
    else {
        fillColor = colorct(t1sampnc(pBeatFlash->pPalTex, 0.0f));
    }
    
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, fillColor);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// PulsePlasma


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
} RBLightGeneratorPulsePlasma;


void rbLightGenerationPulsePlasmaFree(void * pData);
void rbLightGenerationPulsePlasmaGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationPulsePlasmaAlloc(RBTexture1 * pPalTex)
{
    RBLightGeneratorPulsePlasma * pPulsePlasma =
        (RBLightGeneratorPulsePlasma *)malloc(
            sizeof (RBLightGeneratorPulsePlasma));
    
    pPulsePlasma->genDef.pData = pPulsePlasma;
    pPulsePlasma->genDef.free = rbLightGenerationPulsePlasmaFree;
    pPulsePlasma->genDef.generate = rbLightGenerationPulsePlasmaGenerate;
    pPulsePlasma->pPalTex = pPalTex;
    
    return &pPulsePlasma->genDef;
}


void rbLightGenerationPulsePlasmaFree(void * pData)
{
    RBLightGeneratorPulsePlasma * pPulsePlasma =
        (RBLightGeneratorPulsePlasma *)pData;
    
    free(pPulsePlasma);
}


void rbLightGenerationPulsePlasmaGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorPulsePlasma * pPulsePlasma =
        (RBLightGeneratorPulsePlasma *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pPulsePlasma);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// PulseGrid


typedef struct
{
    RBLightGenerator genDef;
    RBColor hColor;
    RBColor vColor;
    float hEnergy;
    float vEnergy;
} RBLightGeneratorPulseGrid;


void rbLightGenerationPulseGridFree(void * pData);
void rbLightGenerationPulseGridGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationPulseGridAlloc(RBColor hColor,
    RBColor vColor)
{
    RBLightGeneratorPulseGrid * pPulseGrid =
        (RBLightGeneratorPulseGrid *)malloc(
            sizeof (RBLightGeneratorPulseGrid));
    
    pPulseGrid->genDef.pData = pPulseGrid;
    pPulseGrid->genDef.free = rbLightGenerationPulseGridFree;
    pPulseGrid->genDef.generate = rbLightGenerationPulseGridGenerate;
    pPulseGrid->hColor = hColor;
    pPulseGrid->vColor = vColor;
    pPulseGrid->hEnergy = 0.0f;
    pPulseGrid->vEnergy = 0.0f;
    
    return &pPulseGrid->genDef;
}


void rbLightGenerationPulseGridFree(void * pData)
{
    RBLightGeneratorPulseGrid * pPulseGrid =
        (RBLightGeneratorPulseGrid *)pData;
    
    free(pPulseGrid);
}


void rbLightGenerationPulseGridGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorPulseGrid * pPulseGrid =
        (RBLightGeneratorPulseGrid *)pData;
    size_t const fWidth = t2getw(pFrame);
    size_t const fHeight = t2geth(pFrame);
    float centerX = (float)(fWidth - 1) * 0.5f;
    float centerY = (float)(fHeight - 1) * 0.5f;
    RBColor const hColor = pPulseGrid->hColor;
    RBColor const vColor = pPulseGrid->vColor;
    float hSpacing;
    float vSpacing;
    
    pPulseGrid->hEnergy = 0.5f * pPulseGrid->hEnergy +
        0.5f * pAnalysis->bassEnergy;
    pPulseGrid->vEnergy = 0.5f * pPulseGrid->vEnergy +
        0.5f * pAnalysis->trebleEnergy;
    
    hSpacing = 4.0f * (1.0f + 0.5f *
        rbClampF(pPulseGrid->hEnergy, 0.0f, 4.0f));
    vSpacing = 4.0f * (1.0f + 0.5f *
        rbClampF(pPulseGrid->vEnergy, 0.0f, 4.0f));
    
    for(size_t j = 0; j < fHeight; ++j) {
        for(size_t i = 0; i < fWidth; ++i) {
            float u = fmodf(fabsf((float)i - centerX) + hSpacing * 0.75f,
                hSpacing);
            float v = fmodf(fabsf((float)j - centerY) + vSpacing * 0.75f,
                vSpacing);
            RBColor hc;
            RBColor vc;
            
            if(u < 0.5f) {
                hc = cscalef(hColor, u * 2.0f);
            }
            else if(u < 1.5f) {
                hc = hColor;
            }
            else if(u < 2.0f) {
                hc = cscalef(hColor, (2.0f - u) * 2.0f);
            }
            else {
                hc = colori(0, 0, 0, 0);
            }
            
            if(v < 1.0f) {
                vc = cscalef(vColor, v * 1.0f);
            }
            else if(v < 1.5f) {
                vc = vColor;
            }
            else if(v < 2.0f) {
                vc = cscalef(vColor, (2.0f - v) * 2.0f);
            }
            else {
                vc = colori(0, 0, 0, 0);
            }
            
            t2sett(pFrame, i, j, cadd(hc, vc));
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// DashedCircles


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
} RBLightGeneratorDashedCircles;


void rbLightGenerationDashedCirclesFree(void * pData);
void rbLightGenerationDashedCirclesGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationDashedCirclesAlloc(RBTexture1 * pPalTex)
{
    RBLightGeneratorDashedCircles * pDashedCircles =
        (RBLightGeneratorDashedCircles *)malloc(
            sizeof (RBLightGeneratorDashedCircles));
    
    pDashedCircles->genDef.pData = pDashedCircles;
    pDashedCircles->genDef.free = rbLightGenerationDashedCirclesFree;
    pDashedCircles->genDef.generate = rbLightGenerationDashedCirclesGenerate;
    pDashedCircles->pPalTex = pPalTex;
    
    return &pDashedCircles->genDef;
}


void rbLightGenerationDashedCirclesFree(void * pData)
{
    RBLightGeneratorDashedCircles * pDashedCircles =
        (RBLightGeneratorDashedCircles *)pData;
    
    free(pDashedCircles);
}


void rbLightGenerationDashedCirclesGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorDashedCircles * pDashedCircles =
        (RBLightGeneratorDashedCircles *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pDashedCircles);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// SmokeSignals


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
} RBLightGeneratorSmokeSignals;


void rbLightGenerationSmokeSignalsFree(void * pData);
void rbLightGenerationSmokeSignalsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationSmokeSignalsAlloc(RBTexture1 * pPalTex)
{
    RBLightGeneratorSmokeSignals * pSmokeSignals =
        (RBLightGeneratorSmokeSignals *)malloc(
            sizeof (RBLightGeneratorSmokeSignals));
    
    pSmokeSignals->genDef.pData = pSmokeSignals;
    pSmokeSignals->genDef.free = rbLightGenerationSmokeSignalsFree;
    pSmokeSignals->genDef.generate = rbLightGenerationSmokeSignalsGenerate;
    pSmokeSignals->pPalTex = pPalTex;
    
    return &pSmokeSignals->genDef;
}


void rbLightGenerationSmokeSignalsFree(void * pData)
{
    RBLightGeneratorSmokeSignals * pSmokeSignals =
        (RBLightGeneratorSmokeSignals *)pData;
    
    free(pSmokeSignals);
}


void rbLightGenerationSmokeSignalsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorSmokeSignals * pSmokeSignals =
        (RBLightGeneratorSmokeSignals *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pSmokeSignals);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// Fireworks


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pPalTex;
} RBLightGeneratorFireworks;


void rbLightGenerationFireworksFree(void * pData);
void rbLightGenerationFireworksGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationFireworksAlloc(RBTexture1 * pPalTex)
{
    RBLightGeneratorFireworks * pFireworks =
        (RBLightGeneratorFireworks *)malloc(
            sizeof (RBLightGeneratorFireworks));
    
    pFireworks->genDef.pData = pFireworks;
    pFireworks->genDef.free = rbLightGenerationFireworksFree;
    pFireworks->genDef.generate = rbLightGenerationFireworksGenerate;
    pFireworks->pPalTex = pPalTex;
    
    return &pFireworks->genDef;
}


void rbLightGenerationFireworksFree(void * pData)
{
    RBLightGeneratorFireworks * pFireworks =
        (RBLightGeneratorFireworks *)pData;
    
    free(pFireworks);
}


void rbLightGenerationFireworksGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorFireworks * pFireworks =
        (RBLightGeneratorFireworks *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pFireworks);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// VolumeBars


typedef struct
{
    RBLightGenerator genDef;
    RBTexture1 * pLowPalTex;
    RBTexture1 * pHiPalTex;
} RBLightGeneratorVolumeBars;


void rbLightGenerationVolumeBarsFree(void * pData);
void rbLightGenerationVolumeBarsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationVolumeBarsAlloc(RBTexture1 * pLowPalTex,
    RBTexture1 * pHiPalTex)
{
    RBLightGeneratorVolumeBars * pVolumeBars =
        (RBLightGeneratorVolumeBars *)malloc(
            sizeof (RBLightGeneratorVolumeBars));
    
    pVolumeBars->genDef.pData = pVolumeBars;
    pVolumeBars->genDef.free = rbLightGenerationVolumeBarsFree;
    pVolumeBars->genDef.generate = rbLightGenerationVolumeBarsGenerate;
    pVolumeBars->pLowPalTex = pLowPalTex;
    pVolumeBars->pHiPalTex = pHiPalTex;
    
    return &pVolumeBars->genDef;
}


void rbLightGenerationVolumeBarsFree(void * pData)
{
    RBLightGeneratorVolumeBars * pVolumeBars =
        (RBLightGeneratorVolumeBars *)pData;
    
    free(pVolumeBars);
}


void rbLightGenerationVolumeBarsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorVolumeBars * pVolumeBars =
        (RBLightGeneratorVolumeBars *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pVolumeBars);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// BeatStars


typedef struct
{
    RBLightGenerator genDef;
    RBColor color;
} RBLightGeneratorBeatStars;


void rbLightGenerationBeatStarsFree(void * pData);
void rbLightGenerationBeatStarsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationBeatStarsAlloc(RBColor color)
{
    RBLightGeneratorBeatStars * pBeatStars =
        (RBLightGeneratorBeatStars *)malloc(
            sizeof (RBLightGeneratorBeatStars));
    
    pBeatStars->genDef.pData = pBeatStars;
    pBeatStars->genDef.free = rbLightGenerationBeatStarsFree;
    pBeatStars->genDef.generate = rbLightGenerationBeatStarsGenerate;
    pBeatStars->color = color;
    pBeatStars->color = color;
    
    return &pBeatStars->genDef;
}


void rbLightGenerationBeatStarsFree(void * pData)
{
    RBLightGeneratorBeatStars * pBeatStars =
        (RBLightGeneratorBeatStars *)pData;
    
    free(pBeatStars);
}


void rbLightGenerationBeatStarsGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorBeatStars * pBeatStars =
        (RBLightGeneratorBeatStars *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pBeatStars);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// IconCheckerboard


typedef struct
{
    RBLightGenerator genDef;
    RBColor color;
} RBLightGeneratorIconCheckerboard;


void rbLightGenerationIconCheckerboardFree(void * pData);
void rbLightGenerationIconCheckerboardGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationIconCheckerboardAlloc(RBColor color)
{
    RBLightGeneratorIconCheckerboard * pIconCheckerboard =
        (RBLightGeneratorIconCheckerboard *)malloc(
            sizeof (RBLightGeneratorIconCheckerboard));
    
    pIconCheckerboard->genDef.pData = pIconCheckerboard;
    pIconCheckerboard->genDef.free = rbLightGenerationIconCheckerboardFree;
    pIconCheckerboard->genDef.generate = rbLightGenerationIconCheckerboardGenerate;
    pIconCheckerboard->color = color;
    
    return &pIconCheckerboard->genDef;
}


void rbLightGenerationIconCheckerboardFree(void * pData)
{
    RBLightGeneratorIconCheckerboard * pIconCheckerboard =
        (RBLightGeneratorIconCheckerboard *)pData;
    
    free(pIconCheckerboard);
}


void rbLightGenerationIconCheckerboardGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorIconCheckerboard * pIconCheckerboard =
        (RBLightGeneratorIconCheckerboard *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pIconCheckerboard);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// PulseCheckerboard


typedef struct
{
    RBLightGenerator genDef;
    RBColor color;
} RBLightGeneratorPulseCheckerboard;


void rbLightGenerationPulseCheckerboardFree(void * pData);
void rbLightGenerationPulseCheckerboardGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationPulseCheckerboardAlloc(RBColor color)
{
    RBLightGeneratorPulseCheckerboard * pPulseCheckerboard =
        (RBLightGeneratorPulseCheckerboard *)malloc(
            sizeof (RBLightGeneratorPulseCheckerboard));
    
    pPulseCheckerboard->genDef.pData = pPulseCheckerboard;
    pPulseCheckerboard->genDef.free = rbLightGenerationPulseCheckerboardFree;
    pPulseCheckerboard->genDef.generate = rbLightGenerationPulseCheckerboardGenerate;
    pPulseCheckerboard->color = color;
    
    return &pPulseCheckerboard->genDef;
}


void rbLightGenerationPulseCheckerboardFree(void * pData)
{
    RBLightGeneratorPulseCheckerboard * pPulseCheckerboard =
        (RBLightGeneratorPulseCheckerboard *)pData;
    
    free(pPulseCheckerboard);
}


void rbLightGenerationPulseCheckerboardGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorPulseCheckerboard * pPulseCheckerboard =
        (RBLightGeneratorPulseCheckerboard *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pPulseCheckerboard);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// ParticleLissajous


typedef struct
{
    RBLightGenerator genDef;
    RBColor color;
} RBLightGeneratorParticleLissajous;


void rbLightGenerationParticleLissajousFree(void * pData);
void rbLightGenerationParticleLissajousGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGenerationParticleLissajousAlloc(RBColor color)
{
    RBLightGeneratorParticleLissajous * pParticleLissajous =
        (RBLightGeneratorParticleLissajous *)malloc(
            sizeof (RBLightGeneratorParticleLissajous));
    
    pParticleLissajous->genDef.pData = pParticleLissajous;
    pParticleLissajous->genDef.free = rbLightGenerationParticleLissajousFree;
    pParticleLissajous->genDef.generate = rbLightGenerationParticleLissajousGenerate;
    pParticleLissajous->color = color;
    
    return &pParticleLissajous->genDef;
}


void rbLightGenerationParticleLissajousFree(void * pData)
{
    RBLightGeneratorParticleLissajous * pParticleLissajous =
        (RBLightGeneratorParticleLissajous *)pData;
    
    free(pParticleLissajous);
}


void rbLightGenerationParticleLissajousGenerate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGeneratorParticleLissajous * pParticleLissajous =
        (RBLightGeneratorParticleLissajous *)pData;
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pParticleLissajous);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// SignalLissajous


typedef struct
{
    RBLightGenerator genDef;
    RBColor color;
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
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pSignalLissajous);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// Oscilloscope


typedef struct
{
    RBLightGenerator genDef;
    RBColor color;
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
    
    // TODO Implement
    UNUSED(pAnalysis);
    UNUSED(pOscilloscope);
    for(size_t j = 0; j < t2geth(pFrame); ++j) {
        for(size_t i = 0; i < t2getw(pFrame); ++i) {
            t2sett(pFrame, i, j, colori(255, 0, 255, 255));
        }
    }
}

/*
typedef struct
{
    RBLightGenerator genDef;
} RBLightGenerator<**>;


void rbLightGeneration<**>Free(void * pData);
void rbLightGeneration<**>Generate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame);


RBLightGenerator * rbLightGeneration<**>Alloc(void)
{
    RBLightGenerator<**> * p<**> =
        (RBLightGenerator<**> *)malloc(
            sizeof (RBLightGenerator<**>));
    
    p<**>->genDef.pData = p<**>;
    p<**>->genDef.free = rbLightGeneration<**>Free;
    p<**>->genDef.generate = rbLightGeneration<**>Generate;
    
    return &p<**>->genDef;
}


void rbLightGeneration<**>Free(void * pData)
{
    RBLightGenerator<**> * p<**> =
        (RBLightGenerator<**> *)pData;
}


void rbLightGeneration<**>Generate(void * pData,
    RBAnalyzedAudio const * pAnalysis, RBTexture2 * pFrame)
{
    RBLightGenerator<**> * p<**> =
        (RBLightGenerator<**> *)pData;
}


*/


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
