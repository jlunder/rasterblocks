#include "rasterblocks.h"

#include <time.h>

#include "audio_input.h"
#include "audio_analysis.h"
#include "configuration.h"
#include "light_generation.h"


static char const * const g_slLogLevelNames[RBLL_COUNT] = {
    "INFO",
    "WARN",
    "ERR"
};

static char const * const g_slSubsystemNames[RBS_COUNT] = {
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

static RBLogLevel g_slSubsystemLogLevels[RBS_COUNT];

static RBConfiguration g_slConfiguration;
static RBSubsystem g_slCurrentSubsystem = RBS_MAIN;

static bool g_slGentleRestartRequested = false;
static bool g_slDelayGentleRestart = false;
static bool g_slIsRestarting = false;

static RBLightData g_slLastFrameLightData;

static uint64_t g_slClockNs = 0;
static uint64_t g_slClockMsNsRemainder = 0;
static RBTime g_slClockMs = 0;


void rbProcessSubsystems(bool * pCoslClockNsnfigChanged);
void rbProcessConfigChanged(void);
void rbProcessGentleRestart(void);


RBSubsystem rbChangeSubsystem(RBSubsystem subsystem)
{
    RBSubsystem lastSubsystem = g_slCurrentSubsystem;
    
    rbVerify(subsystem + 1 >= 1 && subsystem < RBS_COUNT);
    g_slCurrentSubsystem = subsystem;
    return lastSubsystem;
}


void rbRequestGentleRestart(void)
{
    g_slGentleRestartRequested = true;
}


void rbRequestDelayedGentleRestart(void)
{
    g_slDelayGentleRestart = true;
    g_slGentleRestartRequested = true;
}


void rbRequestImmediateRestart(void)
{
    abort();
}


bool rbIsRestarting(void)
{
    return g_slIsRestarting;
}


RBTime rbGetTime(void)
{
    return g_slClockMs;
}


void rbStartTimer(RBTimer * pTimer, RBTime period)
{
    pTimer->time = rbGetTime();
    pTimer->period = period;
    
    rbAssert(period >= 0);
}


int32_t rbGetTimerPeriods(RBTimer * pTimer)
{
    if(pTimer->period == 0) {
        return 1;
    }
    return rbDiffTime(rbGetTime(), pTimer->time) / pTimer->period;
}


int32_t rbGetTimerPeriodsAndReset(RBTimer * pTimer)
{
    RBTime currentTime = rbGetTime();
    RBTime diff = rbDiffTime(currentTime, pTimer->time);
    RBTime period = pTimer->period;
    RBTime periodCount;
    
    if(period == 0) {
        return 1;
    }
    
    rbAssert(diff >= 0);
    
    // Division is rbow, only do it if the diff is large
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
    
    rbAssert(periodCount == rbGetTimerPeriods(pTimer));
    rbAssert(diff < pTimer->period);
    rbAssert(diff >= 0);
    
    pTimer->time = currentTime - diff;
    
    return periodCount;
}


RBTime rbGetTimeLeft(RBTimer * pTimer)
{
    RBTime diff = rbDiffTime(pTimer->time + pTimer->period, rbGetTime());
    
    if(diff < 0) {
        return 0;
    }
    else {
        return diff;
    }
}


bool rbLogShouldLog(RBLogLevel level, char const * sourceFile, int sourceLine)
{
    UNUSED(sourceFile);
    UNUSED(sourceLine);
    rbAssert(g_slCurrentSubsystem + 1 >= 1 && g_slCurrentSubsystem < RBS_COUNT);
    return level >= g_slSubsystemLogLevels[g_slCurrentSubsystem];
}


void rbLog(RBLogLevel level, char const * sourceFile, int sourceLine,
    char const * format, ...)
{
    if(rbLogShouldLog(level, sourceFile, sourceLine)) {
        char const * logLevelName = "???";
        char const * subsystemName = "???";
        va_list va;
        
        if(level + 1 >= 1 && level < RBLL_COUNT) {
            logLevelName = g_slLogLevelNames[level];
        }
        if(g_slCurrentSubsystem + 1 >= 1 &&
                g_slCurrentSubsystem < RBS_COUNT) {
            subsystemName = g_slSubsystemNames[g_slCurrentSubsystem];
        }
        
        rbLogOutput("%s[%s]:", logLevelName, subsystemName);
        
        va_start(va, format);
        rbLogOutputV(format, va);
        va_end(va);
    }
}


void rbLogOutput(char const * format, ...)
{
    va_list va;
    
    va_start(va, format);
    rbLogOutputV(format, va);
    va_end(va);
}


void rbInitialize(int argc, char * argv[])
{
    RBSubsystem lastSubsystem = rbChangeSubsystem(RBS_MAIN);
    bool reinitFrameData = !g_slGentleRestartRequested;
    RBLogLevel logLevel = RBLL_WARNING;
    
    // Init global variables
    g_slSavedArgc = argc;
    g_slSavedArgv = argv;
    
    if(rbIsRestarting()) {
    }
    
    for(int i = 0; i < argc; ++i) {
        if(strcmp(argv[i], "-v") == 0) {
            logLevel = RBLL_INFO;
        }
    }
    
    for(size_t i = 0; i < RBS_COUNT; ++i) {
        g_slSubsystemLogLevels[i] = logLevel;
    }
    
    rbChangeSubsystem(RBS_CONFIGURATION);
    rbConfigurationSetDefaults(&g_slConfiguration);
    rbConfigurationParseArgv(&g_slConfiguration, argc, argv);
    
    if(reinitFrameData) {
        memset(&g_slLastFrameLightData, 0, sizeof g_slLastFrameLightData);
    }
    
    rbChangeSubsystem(RBS_CONFIGURATION);
    rbConfigurationLoad(&g_slConfiguration);
    // Command-line params should override config file
    rbConfigurationParseArgv(&g_slConfiguration, argc, argv);
    
    for(size_t i = 0; i < RBS_COUNT; ++i) {
        g_slSubsystemLogLevels[i] = g_slConfiguration.logLevel;
    }
    
    rbChangeSubsystem(RBS_AUDIO_INPUT);
    rbAudioInputInitialize(&g_slConfiguration);
    
    rbChangeSubsystem(RBS_AUDIO_ANALYSIS);
    rbAudioAnalysisInitialize(&g_slConfiguration);
    
    rbChangeSubsystem(RBS_LIGHT_GENERATION);
    rbLightGenerationInitialize(&g_slConfiguration);
    
    rbChangeSubsystem(RBS_LIGHT_OUTPUT);
    rbLightOutputInitialize(&g_slConfiguration);
    
    rbChangeSubsystem(RBS_HOT_CONFIGURATION);
    rbHotConfigurationInitialize(&g_slConfiguration);
    
    rbChangeSubsystem(RBS_MAIN);
    // If something goes wrong during init, reinit'ing won't help -- that
    // should be a fatal error!
    rbAssert(!g_slGentleRestartRequested);
    
    rbChangeSubsystem(lastSubsystem);
}


void rbShutdown(void)
{
    RBSubsystem lastSubsystem = rbChangeSubsystem(RBS_MAIN);
    
    rbAssert(lastSubsystem == RBS_MAIN);
    
    rbChangeSubsystem(RBS_HOT_CONFIGURATION);
    rbHotConfigurationShutdown();
    
    rbChangeSubsystem(RBS_LIGHT_OUTPUT);
    rbLightOutputShutdown();
    
    rbChangeSubsystem(RBS_LIGHT_GENERATION);
    rbLightGenerationShutdown();
    
    rbChangeSubsystem(RBS_AUDIO_ANALYSIS);
    rbAudioAnalysisShutdown();
    
    rbChangeSubsystem(RBS_AUDIO_INPUT);
    rbAudioInputShutdown();
    
    rbChangeSubsystem(lastSubsystem);
}


void rbProcess(uint64_t nsSinceLastProcess)
{
    RBSubsystem lastSubsystem = rbChangeSubsystem(RBS_MAIN);
    bool configChanged = false;
    int32_t msSinceLastProcess;
    
    g_slClockNs += nsSinceLastProcess;
    msSinceLastProcess = (g_slClockMsNsRemainder + nsSinceLastProcess) /
        1000000;
    g_slClockMsNsRemainder = nsSinceLastProcess -
        msSinceLastProcess * 1000000;
    g_slClockMs += msSinceLastProcess;
    
    rbInfo("Frame time: %lluns\n", msSinceLastProcess);
    
    rbAssert(lastSubsystem == RBS_MAIN);
    
    rbProcessSubsystems(&configChanged);
    
    if(g_slGentleRestartRequested) {
        rbInfo("Gentle restart requested, %d consecutive\n",
            g_slGentleRestartConsecutiveCount);
        
        if(g_slGentleRestartConsecutiveCount == 0) {
            // This is our first gentle restart, capture the time so we can
            // monitor for timeout
            g_slGentleRestartFirstConsecutiveNs = g_slClockNs;
        }
        else if((g_slClockNs - g_slGentleRestartFirstConsecutiveNs) >
                RB_MAX_CONSECUTIVE_GENTLE_RESTART_NS) {
            // We have been continuously restarting for a long time, give up!
            rbFatal("Continuous restart timeout exceeded\n");
            rbRequestImmediateRestart();
        }
        
        ++g_slGentleRestartConsecutiveCount;
        
        g_slGentleRestartRequested = false;
        rbProcessGentleRestart();
    }
    else {
        g_slGentleRestartConsecutiveCount = 0;
        
        if(configChanged) {
            // If the config changes AND restart is requested, it is processed
            // as a restart.
            rbProcessConfigChanged();
        }
    }
    
    rbChangeSubsystem(lastSubsystem);
}


void rbProcessSubsystems(bool * pConfigChanged)
{
    RBSubsystem lastSubsystem = rbChangeSubsystem(RBS_MAIN);
    RBRawAudio rawAudio;
    RBAnalyzedAudio analysis;
    
    rbChangeSubsystem(RBS_AUDIO_INPUT);
    rbAudioInputBlockingRead(&rawAudio);
    
    rbChangeSubsystem(RBS_LIGHT_OUTPUT);
    rbLightOutputShowLights(&g_slLastFrameLightData);
    
    rbChangeSubsystem(RBS_AUDIO_ANALYSIS);
    rbAudioAnalysisAnalyze(&rawAudio, &analysis);
    
    rbChangeSubsystem(RBS_LIGHT_GENERATION);
    rbLightGenerationGenerate(&analysis, &g_slLastFrameLightData);
    
    // Output will happen right after the next audio read, to minimize jitter.
    rbChangeSubsystem(RBS_HOT_CONFIGURATION);
    rbHotConfigurationProcessAndUpdateConfiguration(&g_slConfiguration,
        pConfigChanged);
    
    rbChangeSubsystem(lastSubsystem);
}


void rbProcessConfigChanged(void)
{
    RBSubsystem lastSubsystem = rbChangeSubsystem(RBS_MAIN);
    
    rbConfigurationSave(&g_slConfiguration);
    
    // Command-line params should override config file
    rbConfigurationParseArgv(&g_slConfiguration, g_slSavedArgc, g_slSavedArgv);
    
    rbChangeSubsystem(RBS_AUDIO_INPUT);
    rbAudioInputInitialize(&g_slConfiguration);
    
    rbChangeSubsystem(RBS_AUDIO_ANALYSIS);
    rbAudioAnalysisInitialize(&g_slConfiguration);
    
    rbChangeSubsystem(RBS_LIGHT_GENERATION);
    rbLightGenerationInitialize(&g_slConfiguration);
    
    rbChangeSubsystem(RBS_LIGHT_OUTPUT);
    rbLightOutputInitialize(&g_slConfiguration);
    
    rbChangeSubsystem(RBS_HOT_CONFIGURATION);
    rbHotConfigurationInitialize(&g_slConfiguration);
    
    rbChangeSubsystem(lastSubsystem);
}


void rbProcessGentleRestart(void)
{
    RBSubsystem lastSubsystem = rbChangeSubsystem(RBS_MAIN);
    
    g_slIsRestarting = true;
    rbShutdown();
    
    if(g_slDelayGentleRestart) {
        struct timespec req;
        
        req.tv_sec = (long)(RB_GENTLE_RESTART_DELAY_NS / 1000000000LLU);
        req.tv_nsec = (long)(RB_GENTLE_RESTART_DELAY_NS % 1000000000LLU);
        
        nanosleep(&req, NULL);
        
        g_slDelayGentleRestart = false;
    }
    
    rbInitialize(g_slSavedArgc, g_slSavedArgv);
    g_slIsRestarting = false;
    
    rbChangeSubsystem(lastSubsystem);
}


