#include "light_generation.h"


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
	setBrightness(&lights->left[0], analysis->bassEnergy);
	for(int i = 1; i < SL_NUM_LIGHTS_LEFT; ++i) {
		setBrightness(&lights->left[i], getBrightness(&lights->left[i-1])*0.9);
	}
	
	setBrightness(&lights->right[0], analysis->midEnergy);
	for(int i = 1; i < SL_NUM_LIGHTS_RIGHT; ++i) {
		setBrightness(&lights->right[i], getBrightness(&lights->right[i-1])*0.9);
	}
	
	setBrightness(&lights->overhead[0], analysis->trebleEnergy);
	for(int i = 1; i < SL_NUM_LIGHTS_OVERHEAD; ++i) {
		setBrightness(&lights->overhead[i], getBrightness(&lights->overhead[i-1])*0.9);
	}
}


