#include "light_generation.h"

#include "light_gen/simple_light_gen.h"

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
    UNUSED(analysis);
    
	/*for(size_t i = 0; i < SL_NUM_LIGHTS; ++i) {
		lights->lights[i].r = i * 255 / (SL_NUM_LIGHTS - 1);
		lights->lights[i].g = 255 - i * 255 / (SL_NUM_LIGHTS - 1);
		lights->lights[i].b = i * 255 / (SL_NUM_LIGHTS - 1);
	}*/

    light_gen_bass(analysis,lights,SL_NUM_LIGHTS);
}


