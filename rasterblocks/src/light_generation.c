#include "light_generation.h"

#include "graphics_util.h"


static RBPiecewiseLinearColorSegment g_rbWarmPalette[] = {
    {{  0,   0,   0, 0}, 2},
    {{ 63,   0,   0, 0}, 2},
    {{127,  15,   0, 0}, 2},
    {{255,  63,   0, 0}, 1},
    {{255, 127,   0, 0}, 1},
    {{255, 255,   0, 0}, 1},
    {{255, 255,  255, 0}, 0},
};

static RBPiecewiseLinearColorSegment g_rbColdPalette[] = {
    {{  0,   0,   0, 0}, 1},
    {{  0,  63, 255, 0}, 1},
    {{127,   0,  63, 0}, 1},
    {{  0, 255,  63, 0}, 1},
    {{255, 255, 255, 0}, 0},
};

static RBPiecewiseLinearColorSegment g_rbRainbowPalette[] = {
    {{255,   0,   0, 0}, 1},
    {{  0, 255,   0, 0}, 1},
    {{  0,   0, 255, 0}, 1},
    {{255,   0,   0, 0}, 0},
};

static uint8_t g_rbIcons[][8] = {
#include "icons.h"
};

static RBTime g_rbIconDebounceTime;
static RBTime g_rbIconDisplayTime;

static RBTexture1 * g_rbPWarmTex = NULL;
static RBTexture1 * g_rbPColdTex = NULL;
static RBTexture1 * g_rbPRainbowTex = NULL;

static RBTexture2 * g_rbPTestTex = NULL;

//static size_t g_rbNextIcon = 0;
//static RBTimer g_rbDebounce;
//static RBTimer g_rbIconDisplayTimer;

//static void rbLightGenerationCompositeIcon(RBPanel * pPanel, size_t icon,
//    float a);


void rbLightGenerationInitialize(RBConfiguration const * config)
{
    UNUSED(config);
    
    rbLightGenerationShutdown();
    
    g_rbIconDebounceTime = rbTimeFromMs(50);
    g_rbIconDisplayTime = rbTimeFromMs(500);
    
    g_rbPWarmTex = rbTexture1Alloc(256);
    rbTexture1FillFromPiecewiseLinear(g_rbPWarmTex, g_rbWarmPalette,
        LENGTHOF(g_rbWarmPalette), false);
    
    g_rbPColdTex = rbTexture1Alloc(256);
    rbTexture1FillFromPiecewiseLinear(g_rbPColdTex, g_rbColdPalette,
        LENGTHOF(g_rbColdPalette), false);
    
    g_rbPRainbowTex = rbTexture1Alloc(3);
    rbTexture1FillFromPiecewiseLinear(g_rbPRainbowTex, g_rbRainbowPalette,
        LENGTHOF(g_rbRainbowPalette), true);
    
    g_rbPTestTex = rbTexture2Alloc(8, 8);
    for(size_t i = 0; i < 8; ++i) {
        for(size_t j = 0; j < 8; ++j) {
            rbTexture2SetTexel(g_rbPTestTex, j, i,
                (g_rbIcons[2][i] >> j) & 1 ? colori(31, 31, 31, 0) :
                    colori(0, 0, 0, 0));
        }
    }
}


void rbLightGenerationShutdown(void)
{
    if(g_rbPWarmTex != NULL) {
        rbTexture1Free(g_rbPWarmTex);
        g_rbPWarmTex = NULL;
    }
    if(g_rbPColdTex != NULL) {
        rbTexture1Free(g_rbPColdTex);
        g_rbPColdTex = NULL;
    }
    if(g_rbPRainbowTex != NULL) {
        rbTexture1Free(g_rbPRainbowTex);
        g_rbPRainbowTex = NULL;
    }
    if(g_rbPTestTex != NULL) {
        rbTexture2Free(g_rbPTestTex);
        g_rbPTestTex = NULL;
    }
}


void rbLightGenerationGenerate(RBAnalyzedAudio const * pAnalysis,
    RBTexture2 * pFrame)
{
    float leftTreble = pAnalysis->trebleEnergy * sqrtf(1.0f - pAnalysis->leftRightBalance);
    float rightTreble = pAnalysis->trebleEnergy * sqrtf(pAnalysis->leftRightBalance);
    float bass = pAnalysis->bassEnergy;
    
    UNUSED(g_rbRainbowPalette);

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
    
    for(size_t i = 0; i < RB_PANEL_HEIGHT * 5; ++i) {
        for(size_t j = 0; j < RB_PANEL_WIDTH * 2; ++j) {
            t2sett(pFrame, j, RB_PROJECTION_HEIGHT - 1 - i,
                colorct(
                    t1sampnc(g_rbPWarmTex,
                        bass - i * (2.0f / (RB_PANEL_HEIGHT * 5)))));
        }
    }
    
    t2bltsa(pFrame, 0, 8, 8, 8, g_rbPTestTex, 0, 0);
    
    //rbLightGenerationCompositeIcon(&lights->overhead, 2, 15);
    UNUSED(g_rbIcons);
}


/*
void rbLightGenerationCompositeIcon(RBPanel * panel, size_t icon, float a)
{
    for(size_t i = 0; i < RB_PANEL_HEIGHT; ++i) {
        uint32_t bits = g_rbIcons[icon][i];
        for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            if(bits & 1) {
                RBColor c = panel->data[i][j];
                RBColor inv = {255 - c.r, 255 - c.g, 255 - c.b, 0};
                panel->data[i][j] = cmixf(c, 1.0f - a, inv, a);
            }
            bits = bits >> 1;
        }
    }
}


*/