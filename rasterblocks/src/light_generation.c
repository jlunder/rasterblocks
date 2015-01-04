#include "light_generation.h"

#include "graphics_util.h"


static RBPiecewiseLinearColorSegment g_rbWarmPalette[] = {
    {{  0,   0,   0, 0}, 1},
    {{127,   0,   0, 0}, 1},
    {{127,  95,   0, 0}, 1},
    {{255, 191,   0, 0}, 1},
    {{255, 255,  63, 0}, 0},
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

static RBTexture1 * g_rbPWarmPaletteTex = NULL;
static RBTexture1 * g_rbPColdPaletteTex = NULL;

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
    
    g_rbPWarmPaletteTex = rbTexture1Alloc(256);
    rbTexture1FillFromPiecewiseLinear(g_rbPWarmPaletteTex, g_rbWarmPalette,
        LENGTHOF(g_rbWarmPalette));
    rbTexture1PrepareForSampling(g_rbPWarmPaletteTex);
    g_rbPColdPaletteTex = rbTexture1Alloc(256);
    rbTexture1FillFromPiecewiseLinear(g_rbPColdPaletteTex, g_rbColdPalette,
        LENGTHOF(g_rbColdPalette));
    rbTexture1PrepareForSampling(g_rbPColdPaletteTex);
}


void rbLightGenerationShutdown(void)
{
    if(g_rbPWarmPaletteTex != NULL) {
        rbTexture1Free(g_rbPWarmPaletteTex);
        g_rbPWarmPaletteTex = NULL;
    }
    if(g_rbPColdPaletteTex != NULL) {
        rbTexture1Free(g_rbPColdPaletteTex);
        g_rbPColdPaletteTex = NULL;
    }
}


void rbLightGenerationGenerate(RBAnalyzedAudio const * pAnalysis,
    RBProjectionFrame * pFrame)
{
    float leftTreble = pAnalysis->trebleEnergy * sqrtf(1.0f - pAnalysis->leftRightBalance);
    float rightTreble = pAnalysis->trebleEnergy * sqrtf(pAnalysis->leftRightBalance);
    float bass = pAnalysis->bassEnergy;
    
    UNUSED(g_rbRainbowPalette);

    for(size_t i = 0; i < RB_PROJECTION_HEIGHT; ++i) {
        for(size_t j = 0; j < RB_PROJECTION_WIDTH; ++j) {
            pFrame->proj[i][j] = colori(i * 255 / (RB_PROJECTION_HEIGHT - 1), 0, j * 255 / (RB_PROJECTION_WIDTH - 1), 0);
        }
    }
    
    /*
    UNUSED(bass);
    UNUSED(leftTreble);
    UNUSED(rightTreble);
    for(size_t i = 0; i < RB_PANEL_HEIGHT * 6; ++i) {
        for(size_t j = 0; j < RB_PANEL_WIDTH / 2; ++j) {
            pFrame->proj[i][j] =
                rbColorMakeCT(
                    rbTexture1SampleNearestClamp(g_rbPWarmPaletteTex,
                        ((float)i + 0.5f) / (float)(RB_PANEL_HEIGHT * 6)));
        }
    }
    for(size_t i = 0; i < RB_PANEL_HEIGHT * 6; ++i) {
        for(size_t j = 0; j < RB_PANEL_WIDTH / 2; ++j) {
            pFrame->proj[i][j + RB_PANEL_WIDTH / 2] =
                rbColorMakeCT(
                    rbTexture1SampleNearestClamp(g_rbPColdPaletteTex,
                        ((float)i + 0.5f) / (float)(RB_PANEL_HEIGHT * 6)));
        }
    }
    */
    
    for(size_t i = 0; i < RB_PANEL_HEIGHT; ++i) {
        for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            int32_t k = i < 4 ? (int32_t)j + 3 - i: (int32_t)j - 4 + i;
            pFrame->proj[i][RB_PANEL_WIDTH - 1 - j] =
                rbColorMakeCT(
                    rbTexture1SampleNearestClamp(g_rbPColdPaletteTex,
                        leftTreble - k * (2.0f / RB_PANEL_WIDTH)));
        }
    }
    
    for(size_t i = 0; i < RB_PANEL_HEIGHT; ++i) {
        for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            int32_t k = i < 4 ? (int32_t)j + 3 - i: (int32_t)j - 4 + i;
            pFrame->proj[i][j + RB_PANEL_WIDTH] =
                rbColorMakeCT(
                    rbTexture1SampleNearestClamp(g_rbPColdPaletteTex,
                        rightTreble - k * (2.0f / RB_PANEL_WIDTH)));
        }
    }
    
    for(size_t i = 0; i < RB_PANEL_HEIGHT * 5; ++i) {
        for(size_t j = 0; j < RB_PANEL_WIDTH * 2; ++j) {
            pFrame->proj[RB_PROJECTION_HEIGHT - 1 - i][j] =
                rbColorMakeCT(
                    rbTexture1SampleNearestClamp(g_rbPWarmPaletteTex,
                        bass - i * (2.0f / (RB_PANEL_HEIGHT * 5))));
        }
    }
    
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