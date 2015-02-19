#include "light_generation.h"

#include "graphics_util.h"


static RBPiecewiseLinearColorSegment g_rbPalBlackRedGoldWhiteFS[] = {
    {{  0,   0,   0, 255}, 2},
    {{ 63,   0,   0, 255}, 2},
    {{127,  15,   0, 255}, 2},
    {{255,  63,   0, 255}, 1},
    {{255, 127,   0, 255}, 1},
    {{255, 255,   0, 255}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackRedGoldWhiteFSAlpha[] = {
    {{  0,   0,   0,   0}, 2},
    {{ 63,   0,   0,  57}, 2},
    {{127,  15,   0, 113}, 2},
    {{255,  63,   0, 170}, 1},
    {{255, 127,   0, 198}, 1},
    {{255, 255,   0, 227}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackBlueGreenWhiteFS[] = {
    {{  0,   0,   0, 255}, 1},
    {{  0,  63, 255, 255}, 1},
    {{127,   0,  63, 255}, 1},
    {{  0, 255,  63, 255}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackBlueGreenWhiteFSAlpha[] = {
    {{  0,   0,   0,   0}, 1},
    {{  0,  63, 255,  64}, 1},
    {{127,   0,  63, 128}, 1},
    {{  0, 255,  63, 192}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackWhiteFS[] = {
    {{  0,   0,   0, 255}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackWhiteFSAlpha[] = {
    {{  0,   0,   0,   0}, 1},
    {{255, 255, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalRainbowFS[] = {
    {{255,   0,   0, 255}, 1},
    {{  0, 255,   0, 255}, 1},
    {{  0,   0, 255, 255}, 1},
    {{255,   0,   0, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackGoldFSAlpha[] = {
    {{  0,   0,   0,   0}, 1},
    {{255, 255,  63, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackPaleBlueFSAlpha[] = {
    {{  0,   0,   0,   0}, 1},
    {{ 63,  63, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackPaleGreenFSAlpha[] = {
    {{  0,   0,   0,   0}, 1},
    {{ 63, 255,  63, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackPurpleFSAlpha[] = {
    {{  0,   0,   0,   0}, 1},
    {{127,   0, 255, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalGreenLavenderHS[] = {
    {{  0,   7,   3, 255}, 1},
    {{ 63,   0, 127, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackPurpleRedHS[] = {
    {{  0,   0,   0, 255}, 1},
    {{ 31,   0,  63, 255}, 1},
    {{127,  15,  15, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBluePurpleGreenHS[] = {
    {{  0,  15,  31, 255}, 1},
    {{  0,  63, 127, 255}, 1},
    {{ 15,   7,  31, 255}, 1},
    {{ 31, 127,  15, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlueGoldHS[] = {
    {{  0,  15,  31, 255}, 1},
    {{  0,  63, 127, 255}, 1},
    {{127, 127,  15, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalRedPinkHS[] = {
    {{ 15,   0,   0, 255}, 1},
    {{ 91,   0,   0, 255}, 1},
    {{127,  63,  63, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackRedGoldHS[] = {
    {{  0,   0,   0, 255}, 2},
    {{ 31,   0,   0, 255}, 2},
    {{ 63,   7,   0, 255}, 2},
    {{127,  31,   0, 255}, 1},
    {{127,  63,   0, 255}, 1},
    {{127, 127,  15, 255}, 0},
};

static RBPiecewiseLinearColorSegment g_rbPalBlackWhiteHS[] = {
    {{  0,   0,   0, 255}, 1},
    {{127, 127, 127, 255}, 0},
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

static char const * g_rbVectorHarmonyLogoData =
    "   XXXXX          XXXX    XXXX        XXXX      "
    "    XXXXX         XXXX    XXXX        XXXX      "
    "     XXXXX        XXXX    XXXX        XXXX      "
    "      XXXXX       XXXX    XXXX        XXXX      "
    "       XXXXX      XXXX    XXXX        XXXX      "
    "        XXXXX     XXXX    XXXX        XXXX      "
    "         XXXXX    XXXX    XXXXXXXXXXXXXXXX      "
    "          XXXXX   XXXX    XXXXXXXXXXXXXXXX      "
    "           XXXXX  XXXX    XXXXXXXXXXXXXXXX      "
    "            XXXXX XXXX    XXXXXXXXXXXXXXXX      "
    "             XXXXXXXXX    XXXX        XXXX      "
    "              XXXXXXXX    XXXX        XXXX      "
    "               XXXXXXX    XXXX        XXXX      "
    "                XXXXXX    XXXX        XXXX      "
    "                 XXXXX    XXXX        XXXX      "
    "                  XXXX    XXXX        XXXX      "
    ;

RBTexture1 * g_rbPBlackRedGoldWhiteFSPalTex = NULL;
RBTexture1 * g_rbPBlackRedGoldWhiteFSAlphaPalTex = NULL;
RBTexture1 * g_rbPBlackBlueGreenWhiteFSPalTex = NULL;
RBTexture1 * g_rbPBlackBlueGreenWhiteFSAlphaPalTex = NULL;
RBTexture1 * g_rbPBlackWhiteFSPalTex = NULL;
RBTexture1 * g_rbPBlackWhiteFSAlphaPalTex = NULL;
// This palette is meant to be used with repeat sampling
RBTexture1 * g_rbPRainbowFSPalTex = NULL;
RBTexture1 * g_rbPBlackGoldFSAlphaPalTex = NULL;
RBTexture1 * g_rbPBlackPaleBlueFSAlphaPalTex = NULL;
RBTexture1 * g_rbPBlackPaleGreenFSAlphaPalTex = NULL;
RBTexture1 * g_rbPBlackPurpleFSAlphaPalTex = NULL;
RBTexture1 * g_rbPGreenLavenderHSPalTex = NULL;
RBTexture1 * g_rbPBlackPurpleRedHSPalTex = NULL;
RBTexture1 * g_rbPBluePurpleGreenHSPalTex = NULL;
RBTexture1 * g_rbPBlueGoldHSPalTex = NULL;
RBTexture1 * g_rbPRedPinkHSPalTex = NULL;
RBTexture1 * g_rbPBlackRedGoldHSPalTex = NULL;
RBTexture1 * g_rbPBlackWhiteHSPalTex = NULL;

RBTexture2 * g_rbPAmericanFlagTex = NULL;
RBTexture2 * g_rbPSeqCircLogoTex = NULL;
RBTexture2 * g_rbPSeqCircLogoTex16x8 = NULL;
RBTexture2 * g_rbPSeqCircLogoTex24x12 = NULL;
RBTexture2 * g_rbPSeqCircLogoTex32x16 = NULL;
RBTexture2 * g_rbPVectorHarmonyLogoTex48x16 = NULL;
RBTexture2 * g_rbPVectorHarmonyLogoTex24x8 = NULL;

RBTexture2 * g_rbPIconTexs[LENGTHOF(g_rbIconsData)];

static RBLightGenerator * g_rbPCurrentGenerator = NULL;


static void rbLightGenerationInitializeGenerators(void);


void rbLightGenerationInitialize(RBConfiguration const * config)
{
    UNUSED(config);
    
    rbLightGenerationShutdown();

#define MAKE_PAL(name) \
    do { \
        g_rbP##name##PalTex = rbTexture1Alloc(256); \
        rbTexture1FillFromPiecewiseLinear(g_rbP##name##PalTex, \
            g_rbPal##name, LENGTHOF(g_rbPal##name), false); \
    } while(false)
    
    MAKE_PAL(BlackRedGoldWhiteFS);
    MAKE_PAL(BlackRedGoldWhiteFSAlpha);
    MAKE_PAL(BlackBlueGreenWhiteFS);
    MAKE_PAL(BlackBlueGreenWhiteFSAlpha);
    MAKE_PAL(BlackWhiteFS);
    MAKE_PAL(BlackWhiteFSAlpha);
    //MAKE_PAL(RainbowFS);
    g_rbPRainbowFSPalTex = rbTexture1Alloc(256);
    rbTexture1FillFromPiecewiseLinear(g_rbPRainbowFSPalTex, g_rbPalRainbowFS,
        LENGTHOF(g_rbPalRainbowFS), true);
    MAKE_PAL(BlackGoldFSAlpha);
    MAKE_PAL(BlackPaleBlueFSAlpha);
    MAKE_PAL(BlackPaleGreenFSAlpha);
    MAKE_PAL(BlackPurpleFSAlpha);
    MAKE_PAL(GreenLavenderHS);
    MAKE_PAL(BlackPurpleRedHS);
    MAKE_PAL(BluePurpleGreenHS);
    MAKE_PAL(BlueGoldHS);
    MAKE_PAL(RedPinkHS);
    MAKE_PAL(BlackRedGoldHS);
    MAKE_PAL(BlackWhiteHS);
#undef MAKE_PAL
    
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
            case 'r': c = colori(63, 0, 0, 255); break;
            case 'w': c = colori(63, 63, 63, 255); break;
            case 'b': c = colori(0, 0, 63, 255); break;
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
    g_rbPSeqCircLogoTex24x12 = rbTexture2Alloc(24, 12);
    rbTexture2Rescale(g_rbPSeqCircLogoTex24x12, g_rbPSeqCircLogoTex);
    g_rbPSeqCircLogoTex32x16 = rbTexture2Alloc(32, 16);
    rbTexture2Rescale(g_rbPSeqCircLogoTex32x16, g_rbPSeqCircLogoTex);
    
    g_rbPVectorHarmonyLogoTex48x16 = rbTexture2Alloc(48, 16);
    for(size_t j = 0; j < t2geth(g_rbPVectorHarmonyLogoTex48x16); ++j) {
        for(size_t i = 0; i < t2getw(g_rbPVectorHarmonyLogoTex48x16); ++i) {
            RBColor c;
            switch(g_rbVectorHarmonyLogoData[
                    j * t2getw(g_rbPVectorHarmonyLogoTex48x16) + i]) {
            case ' ': c = colori(0, 0, 0, 0); break;
            default: c = colori(255, 255, 255, 255); break;
            }
            t2sett(g_rbPVectorHarmonyLogoTex48x16, i, j, c);
        }
    }
    
    g_rbPVectorHarmonyLogoTex24x8 = rbTexture2Alloc(24, 8);
    rbTexture2Rescale(g_rbPVectorHarmonyLogoTex24x8,
        g_rbPVectorHarmonyLogoTex48x16);
    
    rbLightGenerationInitializeGenerators();
}


void rbLightGenerationInitializeGenerators(void)
{
    RBLightGenerator * pTopLayerGenerators[] = {
        /*
        rbLightGenerationImageFilterAlloc(
            rbLightGenerationPlasmaAlloc(g_rbPWarmPalTex),
            g_rbPVectorHarmonyLogoTex24x8),
        rbLightGenerationPulseCheckerboardAlloc(colori(127, 127, 127, 255)),
        rbLightGenerationPulseCheckerboardAlloc(colori(127,  63,   0, 255)),
        rbLightGenerationImageFilterAlloc(
            rbLightGenerationBeatFlashAlloc(g_rbPGrayscalePalAlphaTex),
            g_rbPVectorHarmonyLogoTex24x8),
        */
        rbLightGenerationOscilloscopeAlloc(colori(127, 127, 127, 255)),
        /*
        rbLightGenerationBeatStarsAlloc(colori(255, 255, 255, 255)),
        */
    };
    RBLightGenerator * pBottomLayerGenerators[] = {
        /*
        rbLightGenerationPulseGridAlloc(colori(63, 63, 0, 255),
            colori(0, 63, 0, 255)),
        rbLightGenerationDashedCirclesAlloc(g_rbPPal2Tex),
        rbLightGenerationDashedCirclesAlloc(g_rbPPal3Tex),
        */
        rbLightGenerationPlasmaAlloc(g_rbPRedPinkHSPalTex),
    };
    
    UNUSED(pTopLayerGenerators);
    UNUSED(pBottomLayerGenerators);
    /*
extern RBTexture1 * g_rbPBlackRedGoldWhiteFSAlphaPalTex;
extern RBTexture1 * g_rbPBlackBlueGreenWhiteFSAlphaPalTex;
extern RBTexture1 * g_rbPBlackWhiteFSAlphaPalTex;
extern RBTexture1 * g_rbPBlackGoldFSAlphaPalTex;
extern RBTexture1 * g_rbPBlackPaleBlueFSAlphaPalTex;
extern RBTexture1 * g_rbPBlackPaleGreenFSAlphaPalTex;
extern RBTexture1 * g_rbPBlackPurpleFSAlphaPalTex;



extern RBTexture1 * g_rbPGreenLavenderHSPalTex;
extern RBTexture1 * g_rbPBluePurpleGreenHSPalTex;
extern RBTexture1 * g_rbPBlueGoldHSPalTex;
extern RBTexture1 * g_rbPBlackWhiteHSPalTex;

extern RBTexture1 * g_rbPRedPinkHSPalTex;
extern RBTexture1 * g_rbPBlackPurpleRedHSPalTex;
extern RBTexture1 * g_rbPBlackRedGoldHSPalTex;
    */
    rbLightGenerationSetGenerator(
        rbLightGenerationCompositor2Alloc(
            //rbLightGenerationVerticalBarsAlloc(g_rbPBlackPurpleFSAlphaPalTex, 40, rbTimeFromMs(6), rbTimeFromMs(500)),
            rbLightGenerationVolumeBarsAlloc(g_rbPBlackRedGoldWhiteFSAlphaPalTex, g_rbPBlackBlueGreenWhiteFSAlphaPalTex),
            rbLightGenerationSignalLissajousAlloc(colori(0, 255, 0, 255)))
            );
/*
        rbLightGenerationCompositor2Alloc(
            rbLightGenerationTimedRotationAlloc(
                pBottomLayerGenerators, LENGTHOF(pBottomLayerGenerators)),
            rbLightGenerationTimedRotationAlloc(
                pTopLayerGenerators, LENGTHOF(pTopLayerGenerators))));
*/
}


void rbLightGenerationShutdown(void)
{
    rbLightGenerationSetGenerator(NULL);
    
#define FREE_PAL(name) \
    do { \
        if(g_rbP##name##PalTex != NULL) { \
            rbTexture1Free(g_rbP##name##PalTex); \
            g_rbP##name##PalTex = NULL; \
        } \
    } while(false)
    
    FREE_PAL(BlackRedGoldWhiteFS);
    FREE_PAL(BlackRedGoldWhiteFSAlpha);
    FREE_PAL(BlackBlueGreenWhiteFS);
    FREE_PAL(BlackBlueGreenWhiteFSAlpha);
    FREE_PAL(BlackWhiteFS);
    FREE_PAL(BlackWhiteFSAlpha);
    FREE_PAL(RainbowFS);
    FREE_PAL(BlackGoldFSAlpha);
    FREE_PAL(BlackPaleBlueFSAlpha);
    FREE_PAL(BlackPaleGreenFSAlpha);
    FREE_PAL(BlackPurpleFSAlpha);
    FREE_PAL(GreenLavenderHS);
    FREE_PAL(BlackPurpleRedHS);
    FREE_PAL(BluePurpleGreenHS);
    FREE_PAL(BlueGoldHS);
    FREE_PAL(RedPinkHS);
    FREE_PAL(BlackRedGoldHS);
    FREE_PAL(BlackWhiteHS);
#undef FREE_PAL
    
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
