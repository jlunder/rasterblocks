#include "configuration.h"
#include "configuration_json.h"

#include <sys/stat.h>
#include <time.h>

#define RB_DEFAULT_INPUT_ALSA "plughw:1,0"
#define RB_DEFAULT_INPUT_FILE "test/909Tom X1.wav"
#define RB_DEFAULT_INPUT_CONFIG "/var/lib/rasterblocks/config.json"


time_t g_rbConfigFileMTime = 0;


void rbConfigurationSetDefaults(RBConfiguration * config)
{
    config->logLevel = RBLL_WARNING;
    
#ifdef RB_USE_TARGET_HARNESS
    config->audioSource = RBAIS_DEVICE;
    snprintf(config->audioSourceParam, sizeof config->audioSourceParam,
        RB_DEFAULT_INPUT_ALSA);
#else
    config->audioSource = RBAIS_FILE;
    snprintf(config->audioSourceParam, sizeof config->audioSourceParam,
        RB_DEFAULT_INPUT_FILE);
#endif
    //config->configPath[0] = 0;
    snprintf(config->configPath, sizeof config->configPath,
        RB_DEFAULT_INPUT_CONFIG);
    
    config->lowCutoff = 200.0f;
    config->hiCutoff = 300.0f;
    
    config->agcMax = 1e-0f;
    config->agcMin = 1e-2f;
    config->agcStrength = 0.5f;

    config->brightness = 1.0f / 16;
}


void rbConfigurationParseArgv(RBConfiguration * config, int argc,
    char * argv[])
{
    for(int i = 0; i < argc; ++i) {
        if(strcmp(argv[i], "-v") == 0) {
            config->logLevel = RBLL_INFO;
        }
        
        if(strcmp(argv[i], "-sd") == 0) {
            if(i + 1 < argc) {
                ++i;
                config->audioSource = RBAIS_DEVICE;
                snprintf(config->audioSourceParam,
                    sizeof config->audioSourceParam, "%s", argv[i]);
            }
        }
        if(strcmp(argv[i], "-sf") == 0) {
            if(i + 1 < argc) {
                ++i;
                config->audioSource = RBAIS_FILE;
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


void rbConfigurationLoad(RBConfiguration * config)
{
    bool configModified = true;
    
    rbHotConfigurationProcessAndUpdateConfiguration(config, &configModified);
}


void rbConfigurationSave(RBConfiguration const * config)
{
    UNUSED(config);
}


void rbHotConfigurationInitialize(RBConfiguration const * config)
{
    UNUSED(config);
}


void rbHotConfigurationShutdown(void)
{
}


void rbHotConfigurationProcessAndUpdateConfiguration(RBConfiguration * config,
    bool * pConfigurationModified)
{
    if(config->configPath[0]) {
        struct stat statBuf;
        
        if(stat(config->configPath, &statBuf) == 0) {
            if(g_rbConfigFileMTime != statBuf.st_mtime) {
                *pConfigurationModified = true;
                g_rbConfigFileMTime = statBuf.st_mtime;
            }
        }
        // *pConfigurationModified indicates the caller would like to force a
        // config reload for whatever reason.
        if(*pConfigurationModified) {
            if(!*pConfigurationModified) {
                rbWarning("Config changed, rereading\n");
            }
            rbParseJson(config, config->configPath);
            rbInfo("Config audio source: %d\n", config->audioSource);
            rbInfo("Config audio source param: %s\n",
                config->audioSourceParam);
            rbInfo("Config audio playback enabled: %s\n",
                config->monitorAudio ? "true" : "false");
        }
    }
}

