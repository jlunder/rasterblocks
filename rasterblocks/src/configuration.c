#include "configuration.h"
#include "configuration_json.h"

#include <sys/stat.h>
#include <time.h>


#define RB_CONFIGURATION_OVERRIDE_PATH "/etc/rasterblocks.json"


#define RB_DEFAULT_CONFIG_PATH "/var/lib/rasterblocks/config.json"

#define RB_DEFAULT_INPUT_ALSA "plughw:1,0"
#define RB_DEFAULT_INPUT_OPENAL ""
#define RB_DEFAULT_INPUT_FILE "test/909Tom X1.wav"

#define RB_DEFAULT_OUTPUT_PIXELPUSHER "192.168.0.4"
#define RB_DEFAULT_OUTPUT_SPIDEV "/dev/spidev1.0"


#define RB_DEFAULT_PANEL_ARRAY_WIDTH 4
#define RB_DEFAULT_PANEL_ARRAY_HEIGHT 3
#define RB_DEFAULT_NUM_PANELS \
    (RB_DEFAULT_PANEL_ARRAY_WIDTH * RB_DEFAULT_PANEL_ARRAY_HEIGHT)
#define RB_DEFAULT_NUM_LIGHT_STRINGS 4

RBPanelConfig const g_rbDefaultPanelConfigs[RB_DEFAULT_NUM_PANELS] = {
    {{ 8.0f,  0.0f}, { 1.0f,  0.0f}, { 0.0f,  1.0f}},
    {{ 8.0f,  8.0f}, { 1.0f,  0.0f}, { 0.0f,  1.0f}},
    {{ 8.0f, 16.0f}, { 1.0f,  0.0f}, { 0.0f,  1.0f}},
    {{ 7.0f, 23.0f}, {-1.0f,  0.0f}, { 0.0f, -1.0f}},
    {{ 7.0f, 15.0f}, {-1.0f,  0.0f}, { 0.0f, -1.0f}},
    {{ 7.0f,  7.0f}, {-1.0f,  0.0f}, { 0.0f, -1.0f}},
    
    {{24.0f,  0.0f}, { 1.0f,  0.0f}, { 0.0f,  1.0f}},
    {{24.0f,  8.0f}, { 1.0f,  0.0f}, { 0.0f,  1.0f}},
    {{24.0f, 16.0f}, { 1.0f,  0.0f}, { 0.0f,  1.0f}},
    {{23.0f, 23.0f}, {-1.0f,  0.0f}, { 0.0f, -1.0f}},
    {{23.0f, 15.0f}, {-1.0f,  0.0f}, { 0.0f, -1.0f}},
    {{23.0f,  7.0f}, {-1.0f,  0.0f}, { 0.0f, -1.0f}},
};


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
    pConfig->controlInput = RBCI_PRUSS_MIDI;
#elif defined RB_USE_BBB_UART4_MIDI
    pConfig->controlInput = RBCI_BBB_UART4_MIDI;
#else
    pConfig->controlInput = RBCI_HARNESS;
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

    pConfig->brightness = 1.0f;
    pConfig->projectionWidth = RB_PANEL_WIDTH * RB_DEFAULT_PANEL_ARRAY_WIDTH;
    pConfig->projectionHeight = RB_PANEL_HEIGHT * RB_DEFAULT_PANEL_ARRAY_HEIGHT;
    pConfig->numLightStrings = RB_DEFAULT_NUM_LIGHT_STRINGS;
    pConfig->numLightsPerString = pConfig->projectionWidth *
        pConfig->projectionHeight / pConfig->numLightStrings;
    rbComputeLightPositionsFromPanelList(pConfig->lightPositions,
        RB_MAX_LIGHTS, g_rbDefaultPanelConfigs, RB_DEFAULT_NUM_PANELS);

    pConfig->mode = 0;
}


void rbConfigurationParseArgv(RBConfiguration * pConfig, int argc,
    char * argv[])
{
    for(int i = 0; i < argc; ++i) {
        if(strcmp(argv[i], "-v") == 0) {
            pConfig->logLevel = RBLL_INFO;
        }
        
        else if(strcmp(argv[i], "-fc") == 0) {
            if(i + 1 < argc) {
                ++i;
                snprintf(pConfig->configPath,
                    sizeof pConfig->configPath, "%s", argv[i]);
            }
        }
        
        else if(strcmp(argv[i], "-it") == 0) {
            pConfig->audioInput = RBAI_TEST;
        }
        else if(strcmp(argv[i], "-ia") == 0) {
            if(i + 1 < argc) {
                ++i;
                pConfig->audioInput = RBAI_ALSA;
                rbStrlcpy(pConfig->audioInputParam, argv[i],
                    sizeof pConfig->audioInputParam);
            }
        }
        else if(strcmp(argv[i], "-io") == 0) {
            if(i + 1 < argc) {
                ++i;
                pConfig->audioInput = RBAI_OPENAL;
                rbStrlcpy(pConfig->audioInputParam, argv[i],
                    sizeof pConfig->audioInputParam);
            }
        }
        else if(strcmp(argv[i], "-if") == 0) {
            if(i + 1 < argc) {
                ++i;
                pConfig->audioInput = RBAI_FILE;
                rbStrlcpy(pConfig->audioInputParam, argv[i],
                    sizeof pConfig->audioInputParam);
            }
        }
        else if(strcmp(argv[i], "-ir") == 0) {
            pConfig->audioInput = RBAI_PRUSS;
        }
        
        else if(strcmp(argv[i], "-cn") == 0) {
            pConfig->controlInput = RBCI_NONE;
        }
        else if(strcmp(argv[i], "-ct") == 0) {
            pConfig->controlInput = RBCI_TEST;
        }
        else if(strcmp(argv[i], "-ch") == 0) {
            pConfig->controlInput = RBCI_HARNESS;
        }
        else if(strcmp(argv[i], "-cu") == 0) {
            pConfig->controlInput = RBCI_BBB_UART4_MIDI;
        }
        else if(strcmp(argv[i], "-cr") == 0) {
            pConfig->controlInput = RBCI_PRUSS_MIDI;
        }
        
        else if(strcmp(argv[i], "-og") == 0) {
            pConfig->lightOutput = RBLO_OPENGL;
        }
        else if(strcmp(argv[i], "-op") == 0) {
            if(i + 1 < argc) {
                ++i;
                pConfig->lightOutput = RBLO_PIXELPUSHER;
                rbStrlcpy(pConfig->lightOutputParam, argv[i],
                    sizeof pConfig->lightOutputParam);
            }
        }
        else if(strcmp(argv[i], "-os") == 0) {
            pConfig->lightOutput = RBLO_SPIDEV;
        }
        else if(strcmp(argv[i], "-or") == 0) {
            pConfig->lightOutput = RBLO_PRUSS;
        }
        
        else if(strcmp(argv[i], "-b") == 0) {
            if(i + 1 < argc) {
                ++i;
                pConfig->brightness = atof(argv[i]);
            }
        }
        
        else if(strcmp(argv[i], "-m") == 0) {
            if(i + 1 < argc) {
                ++i;
                pConfig->mode = atoi(argv[i]);
            }
        }
    }
}


void rbConfigurationLoad(RBConfiguration * pConfig)
{
    bool configModified = true;
    
    rbHotConfigurationProcessAndUpdateConfiguration(pConfig, &configModified);
}


void rbConfigurationSave(RBConfiguration const * pConfig)
{
    UNUSED(pConfig);
}


void rbHotConfigurationInitialize(RBConfiguration const * pConfig)
{
    UNUSED(pConfig);
}


void rbHotConfigurationShutdown(void)
{
}


void rbHotConfigurationProcessAndUpdateConfiguration(RBConfiguration * pConfig,
    bool * pConfigurationModified)
{
    if(pConfig->configPath[0]) {
        struct stat statBuf;
        
        if(stat(pConfig->configPath, &statBuf) == 0) {
            if(g_rbConfigFileMTime != statBuf.st_mtime) {
                *pConfigurationModified = true;
                g_rbConfigFileMTime = statBuf.st_mtime;
            }
        }
        // *pConfigurationModified indicates the caller would like to force a
        // pConfig reload for whatever reason.
        if(*pConfigurationModified) {
            if(!*pConfigurationModified) {
                rbWarning("Config changed, rereading\n");
            }
            rbParseJson(pConfig, pConfig->configPath);
            rbParseJson(pConfig, RB_CONFIGURATION_OVERRIDE_PATH);
            rbInfo("Config audio input: %d\n", pConfig->audioInput);
            rbInfo("Config audio input param: %s\n",
                pConfig->audioInputParam);
        }
    }
    else if(*pConfigurationModified) {
        // Force reload.
        rbParseJson(pConfig, RB_CONFIGURATION_OVERRIDE_PATH);
    }
}


