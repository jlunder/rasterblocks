#include "configuration.h"
#include "configuration_json.h"

#define STAGE_LIGHTS_DEFAULT_INPUT_ALSA "plughw:1,0"
#define STAGE_LIGHTS_DEFAULT_INPUT_FILE "../test/clips/909Tom X1.wav"
#define STAGE_LIGHTS_DEFAULT_INPUT_CONFIG "../config/host_config.json"


void slConfigurationSetDefaults(SLConfiguration * config)
{
    config->logLevel = SLLL_WARNING;
    
#ifdef STAGE_LIGHTS_USE_TARGET_HARNESS
    config->audioSource = SLAIS_ALSA;
    snprintf(config->audioSourceParam, sizeof config->audioSourceParam,
        STAGE_LIGHTS_DEFAULT_INPUT_ALSA);
#else
    config->audioSource = SLAIS_FILE;
    snprintf(config->audioSourceParam, sizeof config->audioSourceParam,
        STAGE_LIGHTS_DEFAULT_INPUT_FILE);
#endif
    //config->configPath[0] = 0;
    snprintf(config->configPath, sizeof config->configPath,
        STAGE_LIGHTS_DEFAULT_INPUT_CONFIG);
}


void slConfigurationParseArgv(SLConfiguration * config, int argc,
    char * argv[])
{
    for(int i = 0; i < argc; ++i) {
        if(strcmp(argv[i], "-v") == 0) {
            config->logLevel = SLLL_INFO;
        }
        
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
        if(strcmp(argv[i], "-sc") == 0) {
            if(i + 1 < argc) {
                ++i;
                snprintf(config->configPath,
                    sizeof config->configPath, "%s", argv[i]);
            }
        }
    }
}


void slConfigurationLoad(SLConfiguration * config)
{
    if(config->configPath[0]) {
        slParseJson(config,config->configPath);
    }
    slWarning("Using Audio Source: %d\n",config->audioSource);
    slWarning("Using Audio Source Param: %s\n",config->audioSourceParam);
    slWarning("Audio Playback Enabled: %s\n",config->monitorAudio?"true":"false");
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


