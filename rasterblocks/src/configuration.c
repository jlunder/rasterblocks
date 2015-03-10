#include "configuration.h"
#include "configuration_json.h"

#include <sys/stat.h>
#include <time.h>

#define RB_DEFAULT_CONFIG_PATH "/var/lib/rasterblocks/config.json"

#define RB_DEFAULT_INPUT_ALSA "plughw:1,0"
#define RB_DEFAULT_INPUT_OPENAL ""
#define RB_DEFAULT_INPUT_FILE "test/909Tom X1.wav"

#define RB_DEFAULT_OUTPUT_PIXELPUSHER "192.168.0.4"
#define RB_DEFAULT_OUTPUT_SPIDEV "/dev/spidev1.0"


time_t g_rbConfigFileMTime = 0;


void rbConfigurationSetDefaults(RBConfiguration * pConfig)
{
    pConfig->logLevel = RBLL_WARNING;
    
    //pConfig->pConfigPath[0] = 0;
    rbStrlcpy(pConfig->configPath, RB_DEFAULT_CONFIG_PATH,
        sizeof pConfig->configPath);
    
#if defined RB_USE_ALSA_DEVICE
    pConfig->audioInput = RBAI_ALSA;
    rbStrlcpy(pConfig->audioInputParam, RB_DEFAULT_INPUT_ALSA,
        sizeof pConfig->audioInputParam);
#elif defined RB_USE_OPENAL_DEVICE
    pConfig->audioInput = RBAI_OPENAL;
    rbStrlcpy(pConfig->audioInputParam, RB_DEFAULT_INPUT_OPENAL,
        sizeof pConfig->audioInputParam);
#else
    pConfig->audioInput = RBAI_FILE;
    rbStrlcpy(pConfig->audioInputParam, RB_DEFAULT_INPUT_FILE,
        sizeof pConfig->audioInputParam);
#endif
    
#if defined RB_USE_PRUSS_IO
    pConfig->lightOutput = RBLO_PRUSS;
    rbStrlcpy(pConfig->lightOutputParam, "", sizeof pConfig->lightOutputParam);
#elif defined RB_USE_PIXELPUSHER_OUTPUT
    pConfig->lightOutput = RBLO_PIXELPUSHER;
    rbStrlcpy(pConfig->lightOutputParam, RB_DEFAULT_OUTPUT_PIXELPUSHER,
        sizeof pConfig->lightOutputParam);
#elif defined RB_USE_SPIDEV_OUTPUT
    pConfig->lightOutput = RBLO_SPIDEV;
    rbStrlcpy(pConfig->lightOutputParam, RB_DEFAULT_OUTPUT_SPIDEV,
        sizeof pConfig->lightOutputParam);
#else
    pConfig->lightOutput = RBLO_OPENGL;
    rbStrlcpy(pConfig->lightOutputParam, "", sizeof pConfig->lightOutputParam);
#endif
    
    pConfig->lowCutoff = 200.0f;
    pConfig->hiCutoff = 500.0f;
    
    pConfig->agcMax = 1e-0f;
    pConfig->agcMin = 1e-2f;
    pConfig->agcStrength = 0.5f;

    pConfig->mode = 0;

    pConfig->brightness = 1.0f / 16;
}


void rbConfigurationParseArgv(RBConfiguration * config, int argc,
    char * argv[])
{
    for(int i = 0; i < argc; ++i) {
        if(strcmp(argv[i], "-v") == 0) {
            config->logLevel = RBLL_INFO;
        }
        
        if(strcmp(argv[i], "-c") == 0) {
            if(i + 1 < argc) {
                ++i;
                snprintf(config->configPath,
                    sizeof config->configPath, "%s", argv[i]);
            }
        }
        
        if(strcmp(argv[i], "-it") == 0) {
            config->audioInput = RBAI_TEST;
        }
        if(strcmp(argv[i], "-ia") == 0) {
            if(i + 1 < argc) {
                ++i;
                config->audioInput = RBAI_ALSA;
                rbStrlcpy(config->audioInputParam, argv[i],
                    sizeof config->audioInputParam);
            }
        }
        if(strcmp(argv[i], "-io") == 0) {
            if(i + 1 < argc) {
                ++i;
                config->audioInput = RBAI_OPENAL;
                rbStrlcpy(config->audioInputParam, argv[i],
                    sizeof config->audioInputParam);
            }
        }
        if(strcmp(argv[i], "-if") == 0) {
            if(i + 1 < argc) {
                ++i;
                config->audioInput = RBAI_FILE;
                rbStrlcpy(config->audioInputParam, argv[i],
                    sizeof config->audioInputParam);
            }
        }
        
        if(strcmp(argv[i], "-og") == 0) {
            config->lightOutput = RBLO_OPENGL;
        }
        if(strcmp(argv[i], "-op") == 0) {
            if(i + 1 < argc) {
                ++i;
                config->lightOutput = RBLO_PIXELPUSHER;
                rbStrlcpy(config->lightOutputParam, argv[i],
                    sizeof config->lightOutputParam);
            }
        }
        if(strcmp(argv[i], "-os") == 0) {
            config->lightOutput = RBLO_SPIDEV;
        }
        if(strcmp(argv[i], "-or") == 0) {
            config->lightOutput = RBLO_PRUSS;
        }
        
        if(strcmp(argv[i], "-b") == 0) {
            if(i + 1 < argc) {
                ++i;
                config->brightness = atof(argv[i]);
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
            rbInfo("Config audio input: %d\n", config->audioInput);
            rbInfo("Config audio input param: %s\n",
                config->audioInputParam);
        }
    }
}


