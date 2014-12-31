#include "rasterblocks.h"

#include <time.h>

#include "audio_input.h"
#include "audio_analysis.h"
#include "configuration.h"
#include "graphics_util.h"
#include "light_generation.h"


RBPanelConfig const g_rbPanelConfigs[RB_NUM_PANELS] = {
    // Left column -- sequenced bottom to top:
    {{0.0f, 40.0f}, {1.0f, 0.0f}},
    {{0.0f, 32.0f}, {1.0f, 0.0f}},
    {{0.0f, 24.0f}, {1.0f, 0.0f}},
    {{0.0f, 16.0f}, {1.0f, 0.0f}},
    {{0.0f,  8.0f}, {1.0f, 0.0f}},
    {{0.0f,  0.0f}, {1.0f, 0.0f}},
/*    
    // Right column -- sequenced top to bottom:
    {{8.0f,  0.0f}, {1.0f, 0.0f}},
    {{8.0f,  8.0f}, {1.0f, 0.0f}},
    {{8.0f, 16.0f}, {1.0f, 0.0f}},
    {{8.0f, 24.0f}, {1.0f, 0.0f}},
    {{8.0f, 32.0f}, {1.0f, 0.0f}},
    {{8.0f, 40.0f}, {1.0f, 0.0f}},
*/
};


static char const * const g_rbLogLevelNames[RBLL_COUNT] = {
    "INFO",
    "WARN",
    "ERR"
};

static char const * const g_rbSubsystemNames[RBS_COUNT] = {
    "MAIN",
    "CONFIGURATION",
    "AUDIO_INPUT",
    "AUDIO_ANALYSIS",
    "LIGHT_GENERATION",
    "LIGHT_OUTPUT",
    "HOT_CONFIGURATION",
};


static uint64_t g_rbGentleRestartFirstConsecutiveNs = 0;
static size_t g_rbGentleRestartConsecutiveCount = 0;


static int g_rbSavedArgc = 0;
static char * * g_rbSavedArgv = NULL;

static RBLogLevel g_rbSubsystemLogLevels[RBS_COUNT];

static RBConfiguration g_rbConfiguration;
static RBSubsystem g_rbCurrentSubsystem = RBS_MAIN;

static bool g_rbGentleRestartRequested = false;
static bool g_rbDelayGentleRestart = false;
static bool g_rbIsRestarting = false;

static RBRawLightFrame g_rbLastFrameLightData;

static uint64_t g_rbClockNs = 0;
static uint64_t g_rbClockMsNsRemainder = 0;
static RBTime g_rbClockMs = 0;


static void rbProcessSubsystems(bool * pCoslClockNsnfigChanged);
static void rbProjectLightData(RBProjectionFrame * pProjFrame,
    RBRawLightFrame * pRawFrame);
static void rbProcessConfigChanged(void);
static void rbProcessGentleRestart(void);

RBSubsystem rbChangeSubsystem(RBSubsystem subsystem)
{
    RBSubsystem lastSubsystem = g_rbCurrentSubsystem;
    
    rbVerify(subsystem + 1 >= 1 && subsystem < RBS_COUNT);
    g_rbCurrentSubsystem = subsystem;
    return lastSubsystem;
}


void rbRequestGentleRestart(void)
{
    g_rbGentleRestartRequested = true;
}


void rbRequestDelayedGentleRestart(void)
{
    g_rbDelayGentleRestart = true;
    g_rbGentleRestartRequested = true;
}


void rbRequestImmediateRestart(void)
{
    abort();
}


bool rbIsRestarting(void)
{
    return g_rbIsRestarting;
}


RBTime rbGetTime(void)
{
    return g_rbClockMs;
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
    rbAssert(g_rbCurrentSubsystem + 1 >= 1 && g_rbCurrentSubsystem < RBS_COUNT);
    return level >= g_rbSubsystemLogLevels[g_rbCurrentSubsystem];
}


void rbLog(RBLogLevel level, char const * sourceFile, int sourceLine,
    char const * format, ...)
{
    if(rbLogShouldLog(level, sourceFile, sourceLine)) {
        char const * logLevelName = "???";
        char const * subsystemName = "???";
        va_list va;
        
        if(level + 1 >= 1 && level < RBLL_COUNT) {
            logLevelName = g_rbLogLevelNames[level];
        }
        if(g_rbCurrentSubsystem + 1 >= 1 &&
                g_rbCurrentSubsystem < RBS_COUNT) {
            subsystemName = g_rbSubsystemNames[g_rbCurrentSubsystem];
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
    bool reinitFrameData = !g_rbGentleRestartRequested;
    RBLogLevel logLevel = RBLL_WARNING;
    
    // Init global variables
    g_rbSavedArgc = argc;
    g_rbSavedArgv = argv;
    
    if(rbIsRestarting()) {
    }
    
    for(int i = 0; i < argc; ++i) {
        if(strcmp(argv[i], "-v") == 0) {
            logLevel = RBLL_INFO;
        }
    }
    
    for(size_t i = 0; i < RBS_COUNT; ++i) {
        g_rbSubsystemLogLevels[i] = logLevel;
    }
    
    rbChangeSubsystem(RBS_CONFIGURATION);
    rbConfigurationSetDefaults(&g_rbConfiguration);
    rbConfigurationParseArgv(&g_rbConfiguration, argc, argv);
    
    if(reinitFrameData) {
        memset(&g_rbLastFrameLightData, 0, sizeof g_rbLastFrameLightData);
    }
    
    rbChangeSubsystem(RBS_CONFIGURATION);
    rbConfigurationLoad(&g_rbConfiguration);
    // Command-line params should override config file
    rbConfigurationParseArgv(&g_rbConfiguration, argc, argv);
    
    for(size_t i = 0; i < RBS_COUNT; ++i) {
        g_rbSubsystemLogLevels[i] = g_rbConfiguration.logLevel;
    }
    
    rbChangeSubsystem(RBS_AUDIO_INPUT);
    rbAudioInputInitialize(&g_rbConfiguration);
    
    rbChangeSubsystem(RBS_AUDIO_ANALYSIS);
    rbAudioAnalysisInitialize(&g_rbConfiguration);
    
    rbChangeSubsystem(RBS_LIGHT_GENERATION);
    rbLightGenerationInitialize(&g_rbConfiguration);
    
    rbChangeSubsystem(RBS_LIGHT_OUTPUT);
    rbLightOutputInitialize(&g_rbConfiguration);
    
    rbChangeSubsystem(RBS_HOT_CONFIGURATION);
    rbHotConfigurationInitialize(&g_rbConfiguration);
    
    rbChangeSubsystem(RBS_MAIN);
    // If something goes wrong during init, reinit'ing won't help -- that
    // should be a fatal error!
    rbAssert(!g_rbGentleRestartRequested);
    
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
    
    g_rbClockNs += nsSinceLastProcess;
    msSinceLastProcess = (g_rbClockMsNsRemainder + nsSinceLastProcess) /
        1000000;
    g_rbClockMsNsRemainder = nsSinceLastProcess -
        msSinceLastProcess * 1000000;
    g_rbClockMs += msSinceLastProcess;
    
    rbInfo("Frame time: %lluns\n", msSinceLastProcess);
    
    rbAssert(lastSubsystem == RBS_MAIN);
    
    rbProcessSubsystems(&configChanged);
    
    if(g_rbGentleRestartRequested) {
        rbInfo("Gentle restart requested, %d consecutive\n",
            g_rbGentleRestartConsecutiveCount);
        
        if(g_rbGentleRestartConsecutiveCount == 0) {
            // This is our first gentle restart, capture the time so we can
            // monitor for timeout
            g_rbGentleRestartFirstConsecutiveNs = g_rbClockNs;
        }
        else if((g_rbClockNs - g_rbGentleRestartFirstConsecutiveNs) >
                RB_MAX_CONSECUTIVE_GENTLE_RESTART_NS) {
            // We have been continuously restarting for a long time, give up!
            rbFatal("Continuous restart timeout exceeded\n");
            rbRequestImmediateRestart();
        }
        
        ++g_rbGentleRestartConsecutiveCount;
        
        g_rbGentleRestartRequested = false;
        rbProcessGentleRestart();
    }
    else {
        g_rbGentleRestartConsecutiveCount = 0;
        
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
    RBProjectionFrame frame;
    
    rbChangeSubsystem(RBS_AUDIO_INPUT);
    rbAudioInputBlockingRead(&rawAudio);
    
    rbChangeSubsystem(RBS_LIGHT_OUTPUT);
    rbLightOutputShowLights(&g_rbLastFrameLightData);
    
    rbChangeSubsystem(RBS_AUDIO_ANALYSIS);
    rbAudioAnalysisAnalyze(&rawAudio, &analysis);
    
    rbChangeSubsystem(RBS_LIGHT_GENERATION);
    rbLightGenerationGenerate(&analysis, &frame);
    
    rbChangeSubsystem(RBS_MAIN);
    rbProjectLightData(&frame, &g_rbLastFrameLightData);
    
    // Don't output yet, even though we have a frame of good data now --
    // output will happen right after the next audio read, to minimize jitter.
    
    rbChangeSubsystem(RBS_HOT_CONFIGURATION);
    rbHotConfigurationProcessAndUpdateConfiguration(&g_rbConfiguration,
        pConfigChanged);
    
    rbChangeSubsystem(lastSubsystem);
}


void rbProjectLightData(RBProjectionFrame * pProjFrame,
    RBRawLightFrame * pRawFrame)
{
    for(size_t i = 0; i < RB_NUM_PANELS; ++i) {
        RBVector2 scanPos = g_rbPanelConfigs[i].position;
        RBVector2 xinc = g_rbPanelConfigs[i].orientation;
        RBVector2 yinc = v2cross(xinc);
        
        for(size_t j = 0; j < RB_PANEL_HEIGHT; ++j) {
            // Offset 0.5,0.5 to sample center of texel
            RBVector2 pos = v2add(scanPos, vector2(0.5f, 0.5f));
            
            for(size_t k = 0; k < RB_PANEL_WIDTH; ++k) {
                // Check we are within the backing texture
                rbAssert(pos.x >= 0 && pos.y >= 0 &&
                    pos.x < RB_PROJECTION_WIDTH &&
                    pos.y < RB_PROJECTION_HEIGHT);
                // Nearest-neighbor sampling
                pRawFrame->data[i][j][k] =
                    pProjFrame->proj[(size_t)pos.y][(size_t)pos.x];
                pos = v2add(pos, xinc);
            }
            scanPos = v2add(scanPos, yinc);
        }
    }
}


void rbProcessConfigChanged(void)
{
    RBSubsystem lastSubsystem = rbChangeSubsystem(RBS_MAIN);
    
    rbConfigurationSave(&g_rbConfiguration);
    
    // Command-line params should override config file
    rbConfigurationParseArgv(&g_rbConfiguration, g_rbSavedArgc, g_rbSavedArgv);
    
    rbChangeSubsystem(RBS_AUDIO_INPUT);
    rbAudioInputInitialize(&g_rbConfiguration);
    
    rbChangeSubsystem(RBS_AUDIO_ANALYSIS);
    rbAudioAnalysisInitialize(&g_rbConfiguration);
    
    rbChangeSubsystem(RBS_LIGHT_GENERATION);
    rbLightGenerationInitialize(&g_rbConfiguration);
    
    rbChangeSubsystem(RBS_LIGHT_OUTPUT);
    rbLightOutputInitialize(&g_rbConfiguration);
    
    rbChangeSubsystem(RBS_HOT_CONFIGURATION);
    rbHotConfigurationInitialize(&g_rbConfiguration);
    
    rbChangeSubsystem(lastSubsystem);
}


void rbProcessGentleRestart(void)
{
    RBSubsystem lastSubsystem = rbChangeSubsystem(RBS_MAIN);
    
    g_rbIsRestarting = true;
    rbShutdown();
    
    if(g_rbDelayGentleRestart) {
        struct timespec req;
        
        req.tv_sec = (long)(RB_GENTLE_RESTART_DELAY_NS / 1000000000LLU);
        req.tv_nsec = (long)(RB_GENTLE_RESTART_DELAY_NS % 1000000000LLU);
        
        nanosleep(&req, NULL);
        
        g_rbDelayGentleRestart = false;
    }
    
    rbInitialize(g_rbSavedArgc, g_rbSavedArgv);
    g_rbIsRestarting = false;
    
    rbChangeSubsystem(lastSubsystem);
}

