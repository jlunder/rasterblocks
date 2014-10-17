#include "stage_lights.h"


void slInitialize(int argc, char * argv[])
{
    UNUSED(argc);
    UNUSED(argv);
}


void slShutdown(void)
{
}


void slProcess(uint64_t nsSinceLastProcess)
{
	UNUSED(nsSinceLastProcess);
	slLightOutputShowLights(NULL);
}


void slConfigurationSetDefaults(SLConfiguration * config)
{
    UNUSED(config);
}


void slConfigurationLoad(SLConfiguration * config)
{
    UNUSED(config);
}


void slConfigurationSave(SLConfiguration const * config)
{
    UNUSED(config);
}


void slAudioInputInitialize(SLConfiguration const * config)
{
    UNUSED(config);
}


void slAudioInputShutdown(void)
{
}


void slAudioInputBlockingRead(SLRawAudio * audio)
{
    UNUSED(audio);
}


void slAudioAnalysisInitialize(SLConfiguration const * config)
{
    UNUSED(config);
}


void slAudioAnalysisShutdown(void)
{
}


void slAudioAnalysisAnalyze(SLRawAudio const * audio,
    SLAnalyzedAudio * analysis)
{
    UNUSED(audio);
    UNUSED(analysis);
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
    UNUSED(analysis);
    UNUSED(lights);
}


void slLightOutputInitialize(SLConfiguration const * config)
{
    UNUSED(config);
}


void slLightOutputShutdown(void)
{
}


void slLightOutputShowLights(SLLightData const * lights)
{
    UNUSED(lights);
    
	for(size_t i = 0; i < SL_NUM_LIGHTS; ++i) {
		slHarnessLights[i].r = i * 255 / (SL_NUM_LIGHTS - 1);
		slHarnessLights[i].g = i * 255 / (SL_NUM_LIGHTS - 1);
		slHarnessLights[i].b = i * 255 / (SL_NUM_LIGHTS - 1);
	}
}


void slHotConfigurationInitialize(SLConfiguration const * config)
{
    UNUSED(config);
}


void slHotConfigurationShutdown(void)
{
}


void slHotConfigurationProcessAndUpdateConfiguration(SLConfiguration * config,
    bool * configurationModified)
{
    UNUSED(config);
    UNUSED(configurationModified);
}


