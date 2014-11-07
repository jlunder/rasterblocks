#include "stage_lights.h"

#include "audio_input.h"
#include "audio_analysis.h"
#include "configuration.h"
#include "light_generation.h"


static char const * const g_slLogLevelNames[SLLL_COUNT] = {
    "INFO",
    "WARN",
    "ERR"
};

static char const * const g_slSubsystemNames[SLS_COUNT] = {
    "MAIN",
    "CONFIGURATION",
    "AUDIO_INPUT",
    "AUDIO_ANALYSIS",
    "LIGHT_GENERATION",
    "LIGHT_OUTPUT",
    "HOT_CONFIGURATION",
};


static size_t g_slGentleRestartConsecutiveCount = 0;


static int g_slSavedArgc = 0;
static char * * g_slSavedArgv = NULL;

static SLLogLevel g_slSubsystemLogLevels[SLS_COUNT];

static SLConfiguration g_slConfiguration;
static SLSubsystem g_slCurrentSubsystem = SLS_MAIN;

static bool g_slGentleRestartRequested = false;

static SLLightData g_slLastFrameLightData;


void slProcessSubsystems(bool * pConfigChanged);
void slProcessConfigChanged(void);
void slProcessGentleRestart(void);


SLSubsystem slChangeSubsystem(SLSubsystem subsystem)
{
    SLSubsystem lastSubsystem = g_slCurrentSubsystem;
    
    slVerify(subsystem + 1 >= 1 && subsystem < SLS_COUNT);
    g_slCurrentSubsystem = subsystem;
    return lastSubsystem;
}


void slRequestGentleRestart(void)
{
    g_slGentleRestartRequested = true;
}


void slRequestImmediateRestart(void)
{
    abort();
}


bool slLogShouldLog(SLLogLevel level, char const * sourceFile, int sourceLine)
{
    UNUSED(sourceFile);
    UNUSED(sourceLine);
    slAssert(g_slCurrentSubsystem + 1 >= 1 && g_slCurrentSubsystem < SLS_COUNT);
    return level >= g_slSubsystemLogLevels[g_slCurrentSubsystem];
}


void slLog(SLLogLevel level, char const * sourceFile, int sourceLine,
    char const * format, ...)
{
    if(slLogShouldLog(level, sourceFile, sourceLine)) {
        char const * logLevelName = "???";
        char const * subsystemName = "???";
        va_list va;
        
        if(level + 1 >= 1 && level < SLLL_COUNT) {
            logLevelName = g_slLogLevelNames[level];
        }
        if(g_slCurrentSubsystem + 1 >= 1 && g_slCurrentSubsystem < SLS_COUNT) {
            subsystemName = g_slSubsystemNames[g_slCurrentSubsystem];
        }
        
        slLogOutput("%s[%s]:", logLevelName, subsystemName);
        
        va_start(va, format);
        slLogOutputV(format, va);
        va_end(va);
    }
}


void slLogOutput(char const * format, ...)
{
    va_list va;
    
    va_start(va, format);
    slLogOutputV(format, va);
    va_end(va);
}


void slInitialize(int argc, char * argv[])
{
    SLSubsystem lastSubsystem = slChangeSubsystem(SLS_MAIN);
    bool reinitFrameData = !g_slGentleRestartRequested;
    SLLogLevel logLevel = SLLL_WARNING;
    
    // Init global variables
    g_slSavedArgc = argc;
    g_slSavedArgv = argv;
    
    g_slGentleRestartRequested = false;
    
    for(int i = 0; i < argc; ++i) {
        if(strcmp(argv[i], "-v") == 0) {
            logLevel = SLLL_INFO;
        }
    }
    
    for(size_t i = 0; i < SLS_COUNT; ++i) {
        g_slSubsystemLogLevels[i] = logLevel;
    }
    
    slChangeSubsystem(SLS_CONFIGURATION);
    slConfigurationSetDefaults(&g_slConfiguration);
    slConfigurationParseArgv(&g_slConfiguration, argc, argv);
    if(reinitFrameData) {
        memset(&g_slLastFrameLightData, 0, sizeof g_slLastFrameLightData);
    }
    
    slChangeSubsystem(SLS_CONFIGURATION);
    slConfigurationLoad(&g_slConfiguration);
    
    for(size_t i = 0; i < SLS_COUNT; ++i) {
        g_slSubsystemLogLevels[i] = g_slConfiguration.logLevel;
    }
    
    slChangeSubsystem(SLS_AUDIO_INPUT);
    slAudioInputInitialize(&g_slConfiguration);
    
    slChangeSubsystem(SLS_AUDIO_ANALYSIS);
    slAudioAnalysisInitialize(&g_slConfiguration);
    
    slChangeSubsystem(SLS_LIGHT_GENERATION);
    slLightGenerationInitialize(&g_slConfiguration);
    
    slChangeSubsystem(SLS_LIGHT_OUTPUT);
    slLightOutputInitialize(&g_slConfiguration);
    
    slChangeSubsystem(SLS_HOT_CONFIGURATION);
    slHotConfigurationInitialize(&g_slConfiguration);
    
    slChangeSubsystem(SLS_MAIN);
    // If something goes wrong during init, reinit'ing won't help -- that
    // should be a fatal error!
    slAssert(!g_slGentleRestartRequested);
    
    slChangeSubsystem(lastSubsystem);
}


void slShutdown(void)
{
    SLSubsystem lastSubsystem = slChangeSubsystem(SLS_MAIN);
    
    slAssert(lastSubsystem == SLS_MAIN);
    
    slChangeSubsystem(SLS_HOT_CONFIGURATION);
    slHotConfigurationShutdown();
    
    slChangeSubsystem(SLS_LIGHT_OUTPUT);
    slLightOutputShutdown();
    
    slChangeSubsystem(SLS_LIGHT_GENERATION);
    slLightGenerationShutdown();
    
    slChangeSubsystem(SLS_AUDIO_ANALYSIS);
    slAudioAnalysisShutdown();
    
    slChangeSubsystem(SLS_AUDIO_INPUT);
    slAudioInputShutdown();
    
    slChangeSubsystem(lastSubsystem);
}


void slProcess(uint64_t nsSinceLastProcess)
{
    SLSubsystem lastSubsystem = slChangeSubsystem(SLS_MAIN);
    bool configChanged = false;
    
    UNUSED(nsSinceLastProcess);
    
    slAssert(lastSubsystem == SLS_MAIN);
    
    slProcessSubsystems(&configChanged);
    
    if(!g_slGentleRestartRequested && configChanged) {
        slProcessConfigChanged();
    }
    else if(g_slGentleRestartRequested) {
        ++g_slGentleRestartConsecutiveCount;
        slProcessGentleRestart();
    }
    else {
        g_slGentleRestartConsecutiveCount = 0;
    }
    
    slChangeSubsystem(lastSubsystem);
}


void slProcessSubsystems(bool * pConfigChanged)
{
    SLSubsystem lastSubsystem = slChangeSubsystem(SLS_MAIN);
    SLRawAudio rawAudio;
    SLAnalyzedAudio analysis;
    
    slChangeSubsystem(SLS_AUDIO_INPUT);
    slAudioInputBlockingRead(&rawAudio);
    
    slChangeSubsystem(SLS_LIGHT_OUTPUT);
    slLightOutputShowLights(&g_slLastFrameLightData);
    
    slChangeSubsystem(SLS_AUDIO_ANALYSIS);
    slAudioAnalysisAnalyze(&rawAudio, &analysis);
    
    slChangeSubsystem(SLS_LIGHT_GENERATION);
    slLightGenerationGenerate(&analysis, &g_slLastFrameLightData);
    
    // Output will happen right after the next audio read, to minimize jitter.
    slChangeSubsystem(SLS_HOT_CONFIGURATION);
    slHotConfigurationProcessAndUpdateConfiguration(&g_slConfiguration,
        pConfigChanged);
    
    slChangeSubsystem(lastSubsystem);
}


void slProcessConfigChanged(void)
{
    SLSubsystem lastSubsystem = slChangeSubsystem(SLS_MAIN);
    
    slConfigurationSave(&g_slConfiguration);
    
    slChangeSubsystem(SLS_AUDIO_INPUT);
    slAudioInputInitialize(&g_slConfiguration);
    
    slChangeSubsystem(SLS_AUDIO_ANALYSIS);
    slAudioAnalysisInitialize(&g_slConfiguration);
    
    slChangeSubsystem(SLS_LIGHT_GENERATION);
    slLightGenerationInitialize(&g_slConfiguration);
    
    slChangeSubsystem(SLS_LIGHT_OUTPUT);
    slLightOutputInitialize(&g_slConfiguration);
    
    slChangeSubsystem(SLS_HOT_CONFIGURATION);
    slHotConfigurationInitialize(&g_slConfiguration);
    
    slChangeSubsystem(lastSubsystem);
}


void slProcessGentleRestart(void)
{
    SLSubsystem lastSubsystem = slChangeSubsystem(SLS_MAIN);
    
    slShutdown();
    if(g_slGentleRestartConsecutiveCount >
            SL_MAX_CONSECUTIVE_GENTLE_RESTARTS) {
        slRequestImmediateRestart();
    }
    slInitialize(g_slSavedArgc, g_slSavedArgv);
    
    slChangeSubsystem(lastSubsystem);
}


