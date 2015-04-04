#include "rasterblocks.h"

#include "audio_input.h"
#include "audio_analysis.h"
#include "configuration.h"
#include "control_input.h"
#include "graphics_util.h"
#include "light_generation.h"
#include "light_output.h"
#include "pruss_io.h"


static char const * const g_rbLogLevelNames[RBLL_COUNT] = {
    "INFO",
    "WARN",
    "ERR"
};

static char const * const g_rbSubsystemNames[RBS_COUNT] = {
    "MAIN",
    "CONFIGURATION",
    "CONTROL_INPUT",
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
static void rbProjectLightData(RBTexture2 * pProjFrame,
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


RBConfiguration const * rbGetConfiguration(void)
{
    return &g_rbConfiguration;
}


void rbComputeLightPositionsFromPanelList(RBVector2 * pLightPositions,
    size_t numLightPositions, RBPanelConfig const * pPanelConfigs,
    size_t numPanels)
{
    size_t l = 0;
    
    rbAssert(numLightPositions >=
        numPanels * RB_PANEL_WIDTH * RB_PANEL_HEIGHT);
    
    for(size_t k = 0; k < numPanels; ++k) {
        RBVector2 scanPos = pPanelConfigs[k].position;
        RBVector2 uInc = pPanelConfigs[k].uInc;
        RBVector2 uDec = v2scale(uInc, -1.0f);
        RBVector2 vInc = pPanelConfigs[k].vInc;
        
        for(size_t j = 0; j < RB_PANEL_HEIGHT; j += 2) {
            RBVector2 pos = scanPos;
            
            for(size_t i = 0; i < RB_PANEL_WIDTH; ++i) {
                rbAssert(l < numLightPositions);
                pLightPositions[l++] = pos;
                pos = v2add(pos, uInc);
            }
            if((j + 1) >= RB_PANEL_HEIGHT) {
                break;
            }
            pos = v2add(v2add(pos, vInc), uDec);
            for(size_t i = 0; i < RB_PANEL_WIDTH; ++i) {
                rbAssert(l < numLightPositions);
                pLightPositions[l++] = pos;
                pos = v2add(pos, uDec);
            }
            scanPos = v2add(scanPos, v2scale(vInc, 2.0f));
        }
    }
    
    while(l < numLightPositions) {
        pLightPositions[l++] = vector2(0.0f, 0.0f);
    }
}


RBTime rbGetTime(void)
{
    return g_rbClockMs;
}


void rbSleep(RBTime x)
{
    struct timespec ts;
    
    ts.tv_sec = x / 1000;
    ts.tv_nsec = (x % 1000) * 1000000;
    
    nanosleep(&ts, NULL);
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
    if(pTimer->period == 0) {
        return 0;
    }
    else {
        RBTime diff = rbDiffTime(pTimer->time + pTimer->period, rbGetTime());
    
        if(diff < 0) {
            return 0;
        }
        else {
            return diff;
        }
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
    
    srand(time(NULL));
    
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
        rbZero(&g_rbLastFrameLightData, sizeof g_rbLastFrameLightData);
    }
    
    rbChangeSubsystem(RBS_MAIN);
    rbInfo("Reading config file\n");
    rbChangeSubsystem(RBS_CONFIGURATION);
    rbConfigurationLoad(&g_rbConfiguration);
    // Command-line params should override config file
    rbConfigurationParseArgv(&g_rbConfiguration, argc, argv);
    
    for(size_t i = 0; i < RBS_COUNT; ++i) {
        g_rbSubsystemLogLevels[i] = g_rbConfiguration.logLevel;
    }
    
    rbChangeSubsystem(RBS_MAIN);
#ifdef RB_USE_PRUSS_IO
    rbInfo("Initializing PRUSS I/O\n");
    rbPrussIoInitialize(&g_rbConfiguration);
#endif
    
    rbChangeSubsystem(RBS_MAIN);
    rbInfo("Initializing control input\n");
    rbChangeSubsystem(RBS_CONTROL_INPUT);
    rbControlInputInitialize(&g_rbConfiguration);
    
    rbChangeSubsystem(RBS_MAIN);
    rbInfo("Initializing audio input\n");
    rbChangeSubsystem(RBS_AUDIO_INPUT);
    rbAudioInputInitialize(&g_rbConfiguration);
    
    rbChangeSubsystem(RBS_MAIN);
    rbInfo("Initializing audio analysis\n");
    rbChangeSubsystem(RBS_AUDIO_ANALYSIS);
    rbAudioAnalysisInitialize(&g_rbConfiguration);
    
    rbChangeSubsystem(RBS_MAIN);
    rbInfo("Initializing light generation\n");
    rbChangeSubsystem(RBS_LIGHT_GENERATION);
    rbLightGenerationInitialize(&g_rbConfiguration);
    
    rbChangeSubsystem(RBS_MAIN);
    rbInfo("Initializing light output\n");
    rbChangeSubsystem(RBS_LIGHT_OUTPUT);
    rbLightOutputInitialize(&g_rbConfiguration);
    
    rbChangeSubsystem(RBS_MAIN);
    rbInfo("Initializing hot configuration system\n");
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
    
    rbChangeSubsystem(RBS_CONTROL_INPUT);
    rbControlInputShutdown();
    
#ifdef RB_USE_PRUSS_IO
    rbChangeSubsystem(RBS_MAIN);
    rbPrussIoShutdown();
#endif
    
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
    g_rbClockMsNsRemainder = g_rbClockMsNsRemainder + nsSinceLastProcess -
        (uint64_t)msSinceLastProcess * 1000000;
    rbAssert(g_rbClockMsNsRemainder < 1000000);
    rbAssert(msSinceLastProcess >= 0);
    g_rbClockMs += msSinceLastProcess;
    rbAssert(g_rbClockNs == (uint64_t)g_rbClockMs * 1000000 +
        g_rbClockMsNsRemainder);
    
    rbInfo("Frame time: %dms\n", msSinceLastProcess);
    
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
    RBTexture2 * pFrame =
        rbTexture2Alloc(g_rbConfiguration.projectionWidth,
            g_rbConfiguration.projectionHeight);
    
#ifdef RB_USE_PRUSS_IO
    // Input poll causes the PRUSS I/O system to effectively do a blocking
    // read of audio if the PRUSS audio is enabled, and a non-blocking read
    // of available MIDI data if PRUSS MIDI control input is enabled.
    // If they're both enabled they're read in sync.
    // That these systems are coupled is kind of inevitable: the problem of
    // syncing input is kind of a fundamental one, and it's actually a little
    // bit suboptimal that the UART4 MIDI is decoupled from e.g. the ALSA
    // audio input timing.
    // This should probably be treated in a more universal way; that's
    // something for a future rearchitecting exercise...
    rbChangeSubsystem(RBS_MAIN);
    rbPrussIoReadInput();
#endif
    rbChangeSubsystem(RBS_AUDIO_INPUT);
    rbAudioInputBlockingRead(&rawAudio);
    
    // Do light output and control input right after audio input to stabilize
    // timings
    rbChangeSubsystem(RBS_LIGHT_OUTPUT);
    rbLightOutputShowLights(&g_rbLastFrameLightData);
    
    rbChangeSubsystem(RBS_CONTROL_INPUT);
    rbControlInputRead(&analysis.controls);
    
    // Everything else is bulk data processing, not timing critical
    rbChangeSubsystem(RBS_AUDIO_ANALYSIS);
    rbAudioAnalysisAnalyze(&rawAudio, &analysis);
    
    rbChangeSubsystem(RBS_LIGHT_GENERATION);
    rbLightGenerationGenerate(&analysis, pFrame);
    
    rbChangeSubsystem(RBS_MAIN);
    rbProjectLightData(pFrame, &g_rbLastFrameLightData);
    
    rbTexture2Free(pFrame);
    
    // Don't output yet, even though we have a frame of good data now --
    // output will happen right after the next audio read, to minimize jitter.
    
    rbChangeSubsystem(RBS_HOT_CONFIGURATION);
    rbHotConfigurationProcessAndUpdateConfiguration(&g_rbConfiguration,
        pConfigChanged);
    
    rbChangeSubsystem(lastSubsystem);
}


void rbProjectLightData(RBTexture2 * pProjFrame, RBRawLightFrame * pRawFrame)
{
    float xScale = 1.0f / (t2getw(pProjFrame) - 1);
    float yScale = 1.0f / (t2geth(pProjFrame) - 1);
    
    pRawFrame->numLightStrings = g_rbConfiguration.numLightStrings;
    pRawFrame->numLightsPerString = g_rbConfiguration.numLightsPerString;
    for(size_t i = 0;
            i < pRawFrame->numLightStrings * pRawFrame->numLightsPerString;
            ++i) {
        RBVector2 pos = vector2(
            g_rbConfiguration.lightPositions[i].x * xScale,
            g_rbConfiguration.lightPositions[i].y * yScale);
        pRawFrame->data[i] = colorct(t2samplc(pProjFrame, pos));
    }
}


void rbProcessConfigChanged(void)
{
    RBSubsystem lastSubsystem = rbChangeSubsystem(RBS_MAIN);
    
    rbConfigurationSave(&g_rbConfiguration);
    
    // Command-line params should override config file
    rbConfigurationParseArgv(&g_rbConfiguration, g_rbSavedArgc, g_rbSavedArgv);
    
    rbChangeSubsystem(RBS_CONTROL_INPUT);
    rbControlInputInitialize(&g_rbConfiguration);
    
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


