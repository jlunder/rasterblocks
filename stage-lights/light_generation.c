#include "light_generation.h"


/*
static SLColor g_slVuPalette[5] = {
    {  0, 255,   0, 0},
    {  0, 255,   0, 0},
    {  0, 255,   0, 0},
    {  0, 255,   0, 0},
    {255,   0,   0, 0},
};
*/

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


static inline void setBrightness(SLColor * c, float b)
{
    if(b >= 1.0f) {
        b = 0.999f;
    }
    else if(b < 0.0f) {
        b = 0.0f;
    }
    
    c->r = (uint8_t)(b * 255 + 0.5f);
    c->g = c->r;
    c->b = c->r;
}

static inline float getBrightness(SLColor const * c)
{
    return ((int)c->r + (int)c->g + (int)c->b) / (255.0f * 3);
}


void slLightGenerationGenerate(SLAnalyzedAudio const * analysis,
    SLLightData * lights)
{
    float leftTreble = analysis->trebleEnergy * sqrtf(1.0f - analysis->leftRightBalance);
    float rightTreble = analysis->trebleEnergy * sqrtf(analysis->leftRightBalance);
    float bass = analysis->bassEnergy;
    
	for(int i = 0; i < SL_NUM_LIGHTS_LEFT; ++i) {
		lights->left[i] = getPaletteF(g_slColdPalette, leftTreble * powf(0.7f, i));
	}
	
	for(int i = 0; i < SL_NUM_LIGHTS_RIGHT; ++i) {
		lights->right[i] = getPaletteF(g_slColdPalette, rightTreble * powf(0.7f, i));
	}
	
	for(int i = 0; i < SL_NUM_LIGHTS_OVERHEAD; ++i) {
		lights->overhead[i] = getPaletteF(g_slWarmPalette, bass * powf(0.7f, i));
	}
}


