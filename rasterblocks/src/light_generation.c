#include "light_generation.h"

#include "graphics_util.h"


static RBPalette g_slWarmPalette = {{
    {  0,   0,   0, 0},
    {127,   0,   0, 0},
    {127,  95,   0, 0},
    {255, 191,   0, 0},
    {255, 255,  63, 0},
}};

static RBPalette g_slColdPalette = {{
    {  0,   0,   0, 0},
    {  0,  63, 255, 0},
    {127,   0,  63, 0},
    {  0, 255,  63, 0},
    {255, 255, 255, 0},
}};

static RBPalette g_slRainbowPalette = {{
    {255,   0,   0, 0},
    {255, 255,   0, 0},
    {  0, 255,   0, 0},
    {  0,   0, 255, 0},
    {255,   0,   0, 0},
}};

static uint8_t g_slIcons[][8] = {
#include "icons.h"
};

static RBTime g_slIconDebounceTime;
static RBTime g_slIconDisplayTime;

//static size_t g_slNextIcon = 0;
//static RBTimer g_slDebounce;
//static RBTimer g_slIconDisplayTimer;

static void rbLightGenerationCompositeIcon(RBPanel * panel, size_t icon,
    float a);


void rbLightGenerationInitialize(RBConfiguration const * config)
{
    UNUSED(config);
    
    g_slIconDebounceTime = rbTimeFromMs(50);
    g_slIconDisplayTime = rbTimeFromMs(500);
}


void rbLightGenerationShutdown(void)
{
}


void rbLightGenerationGenerate(RBAnalyzedAudio const * analysis,
    RBLightData * lights)
{
    float leftTreble = analysis->trebleEnergy * sqrtf(1.0f - analysis->leftRightBalance);
    float rightTreble = analysis->trebleEnergy * sqrtf(analysis->leftRightBalance);
    float bass = analysis->bassEnergy;

    UNUSED(g_slRainbowPalette);

    for(size_t i = 0; i < RB_PANEL_HEIGHT; ++i) {
        for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            int32_t k = i < 4 ? (int32_t)j + 3 - i: (int32_t)j - 4 + i;
            lights->left.data[i][RB_PANEL_WIDTH - 1 - j] =
                plookf(&g_slColdPalette,
                    leftTreble - k * (2.0f / RB_PANEL_WIDTH));
        }
    }
    
    for(size_t i = 0; i < RB_PANEL_HEIGHT; ++i) {
        for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            int32_t k = i < 4 ? (int32_t)j + 3 - i: (int32_t)j - 4 + i;
            lights->right.data[i][j] =
                plookf(&g_slColdPalette,
                    rightTreble - k * (2.0f / RB_PANEL_WIDTH));
        }
    }
    
    for(size_t i = 0; i < RB_PANEL_HEIGHT; ++i) {
        for(size_t j = 0; j < RB_PANEL_WIDTH; ++j) {
            lights->overhead.data[RB_PANEL_HEIGHT - 1 - i][j] =
                plookf(&g_slWarmPalette,
                    bass - i * (2.0f / RB_PANEL_HEIGHT));
        }
    }
    
    //rbLightGenerationCompositeIcon(&lights->overhead, 2, 15);
    UNUSED(rbLightGenerationCompositeIcon);
}


void rbLightGenerationCompositeIcon(RBPanel * panel, size_t icon, float a)
{
    for(size_t i = 0; i < RB_PANEL_HEIGHT; ++i) {
        uint32_t bits = g_slIcons[icon][i];
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


