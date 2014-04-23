#include "configuration.h"
#include "configuration_json.h"

#include <sys/stat.h>
#include <time.h>

#define SL_DEFAULT_INPUT_ALSA "plughw:1,0"
#define SL_DEFAULT_INPUT_FILE "../test/clips/909Tom X1.wav"
#define SL_DEFAULT_INPUT_CONFIG "/var/lib/stage-lights/config.json"


time_t g_slConfigFileMTime = 0;


void slConfigurationSetDefaults(SLConfiguration * config)
{
    config->logLevel = SLLL_WARNING;
    
#ifdef SL_USE_TARGET_HARNESS
    config->audioSource = SLAIS_ALSA;
    snprintf(config->audioSourceParam, sizeof config->audioSourceParam,
        SL_DEFAULT_INPUT_ALSA);
#else
    config->audioSource = SLAIS_FILE;
    snprintf(config->audioSourceParam, sizeof config->audioSourceParam,
        SL_DEFAULT_INPUT_FILE);
#endif
    //config->configPath[0] = 0;
    snprintf(config->configPath, sizeof config->configPath,
        SL_DEFAULT_INPUT_CONFIG);
    
    config->brightness = 1.0f;
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
        if(strcmp(argv[i], "-c") == 0) {
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
    bool configModified = true;
    
    slHotConfigurationProcessAndUpdateConfiguration(config, &configModified);
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
    bool * pConfigurationModified)
{
    if(config->configPath[0]) {
        struct stat statBuf;
        
        stat(config->configPath, &statBuf);
        // *pConfigurationModified indicates the caller would like to force a
        // config reload for whatever reason.
        if(*pConfigurationModified ||
                g_slConfigFileMTime != statBuf.st_mtime) {
            if(!*pConfigurationModified) {
                slWarning("Config changed, rereading\n");
            }
            g_slConfigFileMTime = statBuf.st_mtime;
            slParseJson(config, config->configPath);
            *pConfigurationModified = true;
            slInfo("Config audio source: %d\n", config->audioSource);
            slInfo("Config audio source param: %s\n",
                config->audioSourceParam);
            slInfo("Config audio playback enabled: %s\n",
                config->monitorAudio ? "true" : "false");
        }
    }
}


