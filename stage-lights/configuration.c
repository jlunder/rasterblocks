#include "configuration.h"

#define STAGE_LIGHTS_DEFAULT_INPUT_ALSA "plughw:1,0"
#define STAGE_LIGHTS_DEFAULT_INPUT_FILE "../test/clips/909Tom X1.wav"


void slConfigurationSetDefaults(SLConfiguration * config)
{
#ifdef STAGE_LIGHTS_USE_TARGET_HARNESS
    config->audioSource = SLAIS_ALSA;
    snprintf(config->audioSourceParam, sizeof config->audioSourceParam,
        STAGE_LIGHTS_DEFAULT_INPUT_ALSA);
#else
    config->audioSource = SLAIS_FILE;
    snprintf(config->audioSourceParam, sizeof config->audioSourceParam,
        STAGE_LIGHTS_DEFAULT_INPUT_FILE);
#endif
}


void slConfigurationParseArgv(SLConfiguration * config, int argc,
    char * argv[])
{
    for(int i = 0; i < argc; ++i) {
        if(strcmp(argv[i], "-sa") == 0) {
            if(i + 1 < argc) {
                ++i;
                config->audioSource = SLAIS_ALSA;
                snprintf(config->audioSourceParam,
                    sizeof config->audioSourceParam, "%s", argv[i]);
            }
        }
        if(strcmp(argv[i], "-sf") == 0) {
            if(i + 1 < argc) {
                ++i;
                config->audioSource = SLAIS_FILE;
                snprintf(config->audioSourceParam,
                    sizeof config->audioSourceParam, "%s", argv[i]);
            }
        }
    }
}


void slConfigurationLoad(SLConfiguration * config)
{
    UNUSED(config);
}


void slConfigurationSave(SLConfiguration const * config)
{
    UNUSED(config);
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


