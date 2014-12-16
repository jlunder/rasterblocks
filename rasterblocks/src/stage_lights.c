#include "stage_lights.h"

#include <time.h>

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


static uint64_t g_slGentleRestartFirstConsecutiveNs = 0;
static size_t g_slGentleRestartConsecutiveCount = 0;


static int g_slSavedArgc = 0;
static char * * g_slSavedArgv = NULL;

static SLLogLevel g_slSubsystemLogLevels[SLS_COUNT];

static SLConfiguration g_slConfiguration;
static SLSubsystem g_slCurrentSubsystem = SLS_MAIN;

static bool g_slGentleRestartRequested = false;
static bool g_slDelayGentleRestart = false;
static bool g_slIsRestarting = false;

static SLLightData g_slLastFrameLightData;

static uint64_t g_slClockNs = 0;
static uint64_t g_slClockMsNsRemainder = 0;
static SLTime g_slClockMs = 0;


void slProcessSubsystems(bool * pCoslClockNsnfigChanged);
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


void slRequestDelayedGentleRestart(void)
{
    g_slDelayGentleRestart = true;
    g_slGentleRestartRequested = true;
}


void slRequestImmediateRestart(void)
{
    abort();
}


bool slIsRestarting(void)
{
    return g_slIsRestarting;
}


SLTime slGetTime(void)
{
    return g_slClockMs;
}


void slStartTimer(SLTimer * pTimer, SLTime period)
{
    pTimer->time = slGetTime();
    pTimer->period = period;
    
    slAssert(period >= 0);
}


int32_t slGetTimerPeriods(SLTimer * pTimer)
{
    if(pTimer->period == 0) {
        return 1;
    }
    return slDiffTime(slGetTime(), pTimer->time) / pTimer->period;
}


int32_t slGetTimerPeriodsAndReset(SLTimer * pTimer)
{
    SLTime currentTime = slGetTime();
    SLTime diff = slDiffTime(currentTime, pTimer->time);
    SLTime period = pTimer->period;
    SLTime periodCount;
    
    if(period == 0) {
        return 1;
    }
    
    slAssert(diff >= 0);
    
    // Division is slow, only do it if the diff is large
    if(diff > period * 16) {
        periodCount = diff / period;
        diff -= period * periodCount;
    }
    else {
        periodCount = 0;
        while(diff >= period) {
            ++periodCount;
            diff -= period;
        }
    }
    
    slAssert(periodCount == slGetTimerPeriods(pTimer));
    slAssert(diff < pTimer->period);
    slAssert(diff >= 0);
    
    pTimer->time = currentTime - diff;
    
    return periodCount;
}


SLTime slGetTimeLeft(SLTimer * pTimer)
{
    SLTime diff = slDiffTime(pTimer->time + pTimer->period, slGetTime());
    
    if(diff < 0) {
        return 0;
    }
    else {
        return diff;
    }
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
        if(g_slCurrentSubsystem + 1 >= 1 &&
                g_slCurrentSubsystem < SLS_COUNT) {
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
    
    if(slIsRestarting()) {
    }
    
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
    // Command-line params should override config file
    slConfigurationParseArgv(&g_slConfiguration, argc, argv);
    
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
    int32_t msSinceLastProcess;
    
    g_slClockNs += nsSinceLastProcess;
    msSinceLastProcess = (g_slClockMsNsRemainder + nsSinceLastProcess) /
        1000000;
    g_slClockMsNsRemainder = nsSinceLastProcess -
        msSinceLastProcess * 1000000;
    g_slClockMs += msSinceLastProcess;
    
    slInfo("Frame time: %lluns\n", msSinceLastProcess);
    
    slAssert(lastSubsystem == SLS_MAIN);
    
    slProcessSubsystems(&configChanged);
    
    if(g_slGentleRestartRequested) {
        slInfo("Gentle restart requested, %d consecutive\n",
            g_slGentleRestartConsecutiveCount);
        
        if(g_slGentleRestartConsecutiveCount == 0) {
            // This is our first gentle restart, capture the time so we can
            // monitor for timeout
            g_slGentleRestartFirstConsecutiveNs = g_slClockNs;
        }
        else if((g_slClockNs - g_slGentleRestartFirstConsecutiveNs) >
                SL_MAX_CONSECUTIVE_GENTLE_RESTART_NS) {
            // We have been continuously restarting for a long time, give up!
            slFatal("Continuous restart timeout exceeded\n");
            slRequestImmediateRestart();
        }
        
        ++g_slGentleRestartConsecutiveCount;
        
        g_slGentleRestartRequested = false;
        slProcessGentleRestart();
    }
    else {
        g_slGentleRestartConsecutiveCount = 0;
        
        if(configChanged) {
            // If the config changes AND restart is requested, it is processed
            // as a restart.
            slProcessConfigChanged();
        }
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
    
    // Command-line params should override config file
    slConfigurationParseArgv(&g_slConfiguration, g_slSavedArgc, g_slSavedArgv);
    
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
    
    g_slIsRestarting = true;
    slShutdown();
    
    if(g_slDelayGentleRestart) {
        struct timespec req;
        
        req.tv_sec = (long)(SL_GENTLE_RESTART_DELAY_NS / 1000000000LLU);
        req.tv_nsec = (long)(SL_GENTLE_RESTART_DELAY_NS % 1000000000LLU);
        
        nanosleep(&req, NULL);
        
        g_slDelayGentleRestart = false;
    }
    
    slInitialize(g_slSavedArgc, g_slSavedArgv);
    g_slIsRestarting = false;
    
    slChangeSubsystem(lastSubsystem);
}


