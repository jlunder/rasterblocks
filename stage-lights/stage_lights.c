#include "stage_lights.h"


void slInitialize(void)
{
}


void slShutdown(void)
{
}


void slProcess(uint64_t nsSinceLastProcess)
{
	(void)nsSinceLastProcess;
	
	for(size_t i = 0; i < STAGE_LIGHTS_NUM_LIGHTS; ++i) {
		slLights[i].r = i * 255 / (STAGE_LIGHTS_NUM_LIGHTS - 1);
		slLights[i].g = i * 255 / (STAGE_LIGHTS_NUM_LIGHTS - 1);
		slLights[i].b = i * 255 / (STAGE_LIGHTS_NUM_LIGHTS - 1);
	}
}



