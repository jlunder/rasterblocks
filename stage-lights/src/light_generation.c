#include "light_generation.h"


static SLColor g_slWarmPalette[5] = {
    {  0,   0,   0, 0},
    {127,   0,   0, 0},
    {127,  95,   0, 0},
    {255, 191,   0, 0},
    {255, 255,  63, 0},
};

static SLColor g_slColdPalette[5] = {
    {  0,   0,   0, 0},
    {  0,  63, 255, 0},
    { 63,   0,  63, 0},
    {  0, 255,  63, 0},
    {255, 255, 255, 0},
};


static SLColor getPaletteF(SLColor const palette[5], float f)
{
    if(f < 0.0f) {
        return palette[0];
    }
    else if(f < 1.00f) {
        float a = fmodf(f * 4.0f, 1.0f);
        float b = 1.0f - a;
        size_t i = (size_t)(f * 4.0f);
        SLColor c = {
            (uint8_t)(palette[i].r * b + palette[i + 1].r * a),
            (uint8_t)(palette[i].g * b + palette[i + 1].g * a),
            (uint8_t)(palette[i].b * b + palette[i + 1].b * a),
            0
        };
        return c;
    }
    else {
        return palette[4];
    }
}


void slLightGenerationInitialize(SLConfiguration const * config)
{
    UNUSED(config);
}


void slLightGenerationShutdown(void)
{
}


void slLightGenerationGenerate(SLAnalyzedAudio const * analysis,
    SLLightData * lights)
{
    float leftTreble = analysis->trebleEnergy * sqrtf(1.0f - analysis->leftRightBalance);
    float rightTreble = analysis->trebleEnergy * sqrtf(analysis->leftRightBalance);
    float bass = analysis->bassEnergy;

    for(size_t i = 0; i < SL_PANEL_HEIGHT_LEFT; ++i) {
        for(size_t j = 0; j < SL_PANEL_WIDTH_LEFT; ++j) {
            lights->left[i][SL_PANEL_WIDTH_LEFT - 1 - j] =
                getPaletteF(g_slColdPalette,
                    leftTreble - j * (2.0f / SL_PANEL_WIDTH_LEFT));
        }
    }
    
    for(size_t i = 0; i < SL_PANEL_HEIGHT_RIGHT; ++i) {
        for(size_t j = 0; j < SL_PANEL_WIDTH_RIGHT; ++j) {
            lights->right[i][j] =
                getPaletteF(g_slColdPalette,
                    rightTreble - j * (2.0f / SL_PANEL_WIDTH_RIGHT));
        }
    }
    
    for(size_t i = 0; i < SL_PANEL_HEIGHT_LEFT; ++i) {
        for(size_t j = 0; j < SL_PANEL_WIDTH_LEFT; ++j) {
            lights->overhead[SL_PANEL_HEIGHT_OVERHEAD - 1 - i][j] =
                getPaletteF(g_slWarmPalette,
                    bass - i * (2.0f / SL_PANEL_HEIGHT_OVERHEAD));
        }
    }
}


