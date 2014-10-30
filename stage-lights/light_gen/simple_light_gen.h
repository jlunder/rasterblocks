#include <stdio.h>



void set_brightness(SLColor* light, float val) {
	int brightness = val*255;
	light->r = brightness;
	light->g = brightness;
	light->b = brightness;
}

float get_brightness(SLColor* light) {
	return ((float)(light->r+light->g+light->b))/(255*3);
}

void debug_analysis(SLAnalyzedAudio const * analysis)
{
	printf("bass: %f\n",analysis->bassEnergy);
}

void debug_lights(SLLightData * lights, int num_lights)
{
	printf("lights:");
	for(int i = 0; i < num_lights; ++i) {
		printf(" %f",get_brightness(&lights->lights[i]));
	}
	printf("\n");
}

void slLightGenBasicBass(SLAnalyzedAudio const * analysis, SLLightData * lights, int num_lights)
{
	//debug_analysis(analysis);
	set_brightness(&lights->lights[0], get_brightness(&lights->lights[0])/2);

	set_brightness(&lights->lights[0], analysis->bassEnergy+get_brightness(&lights->lights[0]));

	for(int i = 1; i < num_lights; ++i) {
		set_brightness(&lights->lights[i], get_brightness(&lights->lights[i-1])*0.9);
	}

	//debug_lights(lights, num_lights);
}
