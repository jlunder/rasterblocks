#include "rasterblocks.h"

#include "audio_input.h"
#include "audio_analysis.h"
#include "configuration.h"
#include "control_input.h"
#include "graphics_util.h"
#include "light_generation.h"
#include "light_output.h"
#include "parameter_generation.h"
#include "pruss_io.h"
#include "rasterblocks_lua.h"

#ifdef RB_OSX
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif


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
    "LUA",
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

#ifdef RB_LINUX
struct timespec g_rbStartupTs;
#endif
#ifdef RB_OSX
uint64_t g_rbStartupTime;
mach_timebase_info_data_t g_rbTimeBase;
#endif
static uint64_t g_rbClockNs = 0;
static RBTime g_rbClockMs = 0;
static float g_rbDeltaTimeSeconds = 0.0f;

static uint64_t g_rbLastFrameComputeNs = 0;
static uint64_t g_rbPeakFrameComputeNs = 0;
static uint64_t g_rbPeakFrameComputeNsAge = 0;
static float g_rbAverageFrameComputeNs = 0.0f;

static uint32_t g_rbDebugDisplayFrameCounter = 0;

static RBTimer g_rbDebugDisplayLabelTimer;

static RBColor const g_rbDebugColors[10] = {
    {127,   0,   0, 0},
    {255, 127,   0, 0},
    {127, 127,   0, 0},
    {  0, 127,   0, 0},
    {  0, 127, 127, 0},
    {  0,   0, 127, 0},
    { 63,   0,  95, 0},
    {127,   0, 127, 0},
    { 15,  15,  15, 0},
    {127, 127, 127, 0},
};



static void rbProcessSubsystems(bool * pCoslClockNsnfigChanged);
static void rbProjectLightData(RBTexture2 * pProjFrame,
    RBRawLightFrame * pRawFrame);

static void rbOverlayFrameDebugInfo(RBAnalyzedAudio * pAnalysis,
    RBTexture2 * pFrame);
static void rbOverlayFrameAudioDebugInfo(RBAnalyzedAudio * pAnalysis,
    RBTexture2 * pFrame);
static void rbOverlayFrameControlsDebugInfo(RBAnalyzedAudio * pAnalysis,
    RBTexture2 * pFrame);
static void rbOverlayFramePerfMetricsDebugInfo(RBAnalyzedAudio * pAnalysis,
    RBTexture2 * pFrame);
static void rbOverlayFrameProjectionGridDebugInfo(RBAnalyzedAudio * pAnalysis,
    RBTexture2 * pFrame);

static void rbOverlayRawDebugInfo(RBAnalyzedAudio * pAnalysis,
    RBRawLightFrame * pRawFrame);
static void rbOverlayRawIdentifyPixelsDebugInfo(RBAnalyzedAudio * pAnalysis,
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


void rbRequestHardwareReset(void)
{
    system("reboot");
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


float rbGetDeltaTimeSeconds(void)
{
    return g_rbDeltaTimeSeconds;
}


RBTime rbGetRealTime(void)
{
    return (RBTime)(rbGetRealTimeNs() / 1000000);
}


uint64_t rbGetRealTimeNs(void)
{
#ifdef RB_LINUX
    struct timespec ts;
#endif
#ifdef RB_OSX
    uint64_t time;
#endif
    uint64_t timeNs;
    
#ifdef RB_LINUX
    clock_gettime(CLOCK_MONOTONIC, &ts);
    timeNs = (uint64_t)(ts.tv_nsec - g_rbStartupTs.tv_nsec) +
        (uint64_t)(ts.tv_sec - g_rbStartupTs.tv_sec) * 1000000000LLU;
    rbAssert(timeNs < 0x8000000000000000LLU);
#endif
#ifdef RB_OSX
    time = mach_absolute_time();
    timeNs = ((time - g_rbStartupTime) * g_rbTimeBase.numer) /
        g_rbTimeBase.denom;
#endif
    
    return timeNs;
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
    
#ifdef RB_LINUX
    clock_gettime(CLOCK_MONOTONIC, &g_rbStartupTs);
#endif
#ifdef RB_OSX
    mach_timebase_info(&g_rbTimeBase);
    g_rbStartupTime = mach_absolute_time();
#endif
    
    for(int i = 0; i < argc; ++i) {
        if(strcmp(argv[i], "-v") == 0) {
            logLevel = RBLL_INFO;
        }
    }
    
    for(size_t i = 0; i < RBS_COUNT; ++i) {
        g_rbSubsystemLogLevels[i] = logLevel;
    }
    
    g_rbDebugDisplayFrameCounter = 0;
    
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
    rbInfo("Initializing parameter generation\n");
    rbChangeSubsystem(RBS_PARAMETER_GENERATION);
    rbParameterGenerationInitialize(&g_rbConfiguration);
    
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
    rbInfo("Initializing Lua interpreter\n");
    rbChangeSubsystem(RBS_LUA);
    rbLuaInitialize(&g_rbConfiguration);
    
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
    
    rbChangeSubsystem(RBS_LUA);
    rbLuaShutdown();
    
    rbChangeSubsystem(RBS_HOT_CONFIGURATION);
    rbHotConfigurationShutdown();
    
    rbChangeSubsystem(RBS_LIGHT_OUTPUT);
    rbLightOutputShutdown();
    
    rbChangeSubsystem(RBS_LIGHT_GENERATION);
    rbLightGenerationShutdown();
    
    rbChangeSubsystem(RBS_PARAMETER_GENERATION);
    rbParameterGenerationShutdown();
    
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


void rbProcess(void)
{
    RBSubsystem lastSubsystem = rbChangeSubsystem(RBS_MAIN);
    bool configChanged = false;
    uint64_t lastClockNs = g_rbClockNs;
    uint64_t nsSinceLastProcess;
    
    g_rbClockNs = rbGetRealTimeNs();
    g_rbClockMs = (RBTime)(g_rbClockNs / 1000000);
    nsSinceLastProcess = g_rbClockNs - lastClockNs;
    g_rbDeltaTimeSeconds = nsSinceLastProcess * 1.0e-9f;
    
    rbInfo("Frame time: %.3fms\n", (float)nsSinceLastProcess * 1e-6f);
    
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
            rbError("Continuous restart timeout exceeded\n");
            rbRequestHardwareReset();
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
    RBParameters parameters;
    RBTexture2 * pFrame =
        rbTexture2Alloc(g_rbConfiguration.projectionWidth,
            g_rbConfiguration.projectionHeight);
    uint64_t computeStartNs;
    
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
    
    computeStartNs = rbGetRealTimeNs();
    
    // Everything else is bulk data processing, not timing critical
    rbChangeSubsystem(RBS_AUDIO_ANALYSIS);
    rbAudioAnalysisAnalyze(&rawAudio, &analysis);
    
    rbChangeSubsystem(RBS_PARAMETER_GENERATION);
    rbParameterGenerationGenerate(&analysis, &parameters);

    // Only generate lights if they'll actually be visible! Most of the debug
    // overlays fully take over the display.
    if((analysis.controls.debugDisplayMode == RBDM_OFF) ||
            (analysis.controls.debugDisplayMode == RBDM_PERF_METRICS)) {
        rbChangeSubsystem(RBS_LIGHT_GENERATION);
        rbLightGenerationGenerate(&analysis, &parameters, pFrame);
    }
    
    rbChangeSubsystem(RBS_MAIN);
    if(analysis.controls.debugDisplayReset) {
        g_rbDebugDisplayFrameCounter = 0;
        rbStartTimer(&g_rbDebugDisplayLabelTimer, rbTimeFromMs(4000));
    }
    rbOverlayFrameDebugInfo(&analysis, pFrame);
    rbProjectLightData(pFrame, &g_rbLastFrameLightData);
    // This is kind of a hacky way to propagate the frame number; perhaps this
    // is an indication we should be wrapping the pFrame texture in another
    // purpose-specific struct so ...Generate can pass it along?
    g_rbLastFrameLightData.frameNum = analysis.frameNum;
    rbAssert(g_rbLastFrameLightData.frameNum == rawAudio.frameNum);
    rbOverlayRawDebugInfo(&analysis, &g_rbLastFrameLightData);
    ++g_rbDebugDisplayFrameCounter;
    
    rbTexture2Free(pFrame);
    
    if(g_rbPeakFrameComputeNsAge > RB_VIDEO_FRAME_RATE * 1) {
        g_rbPeakFrameComputeNs = 0;
    }
    g_rbLastFrameComputeNs = rbGetRealTimeNs() - computeStartNs;
    g_rbAverageFrameComputeNs = g_rbAverageFrameComputeNs * 0.999f +
        (float)g_rbLastFrameComputeNs * 0.001f;
    if(g_rbLastFrameComputeNs >= g_rbPeakFrameComputeNs) {
        g_rbPeakFrameComputeNs = g_rbLastFrameComputeNs;
        g_rbPeakFrameComputeNsAge = 0;
    }
    ++g_rbPeakFrameComputeNsAge;
    
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


void rbOverlayFrameDebugInfo(RBAnalyzedAudio * pAnalysis, RBTexture2 * pFrame)
{
    char const * modeName = NULL;
    
    switch(pAnalysis->controls.debugDisplayMode) {
    case RBDM_OFF:
        modeName = "Run";
        break;
    case RBDM_AUDIO:
        modeName = "Audio";
        rbOverlayFrameAudioDebugInfo(pAnalysis, pFrame);
        break;
    case RBDM_CONTROLS:
        modeName = "Controls";
        rbOverlayFrameControlsDebugInfo(pAnalysis, pFrame);
        break;
    case RBDM_PERF_METRICS:
        modeName = "Perf";
        rbOverlayFramePerfMetricsDebugInfo(pAnalysis, pFrame);
        break;
    case RBDM_PROJECTION_GRID:
        modeName = "Grid";
        rbOverlayFrameProjectionGridDebugInfo(pAnalysis, pFrame);
        break;
    case RBDM_IDENTIFY_PIXELS:
        break;
    default:
        rbFatal("Invalid debug display mode??\n");
        break;
    }
    
    if(modeName != NULL && rbGetTimeLeft(&g_rbDebugDisplayLabelTimer) > 0) {
        uint8_t a = 255;
        if(rbGetTimeLeft(&g_rbDebugDisplayLabelTimer) < rbTimeFromMs(2000)) {
            a = (uint8_t)(rbGetTimeLeft(&g_rbDebugDisplayLabelTimer) *
                255.0f / rbTimeFromMs(2000));
        }
        t2rect(pFrame, 0, 0, t2getw(pFrame), RB_DEBUG_CHAR_HEIGHT,
            colori(0, 0, 0, a));
        t2dtextf(pFrame, 0, 0, colori(a, a, a, a),
            modeName);
    }
}


void rbOverlayFrameAudioDebugInfo(RBAnalyzedAudio * pAnalysis,
    RBTexture2 * pFrame)
{
    size_t w = t2getw(pFrame);
    size_t h = t2geth(pFrame);
    size_t const oscopeH = h * 4 / 8;
    RBColor const bgC = colori(0, 0, 0, 0);
    RBColor const agcC = colori(127, 127, 0, 0);
    RBColor const lrBalC = colori(0, 127, 63, 0);
    RBColor const bassC = colori(127, 63, 0, 0);
    RBColor const trebC = colori(0, 0, 127, 0);
    RBColor const odC = colori(255, 0, 0, 0);
    RBColor const ldcC = colori(127, 0, 127, 0);
    RBColor const pdC = colori(0, 0, 255, 0);
    RBColor const oscopeC = colori(63, 255, 15, 0);
    float mins[w];
    float maxs[w];
    float avg = 0.0f;
    
    for(size_t i = 0; i < w; ++i) {
        mins[i] = 1.0f;
        maxs[i] = 0.0f;
    }
    rbAssert(w <= RB_AUDIO_FRAMES_PER_VIDEO_FRAME);
    for(size_t j = 0; j < RB_AUDIO_FRAMES_PER_VIDEO_FRAME; ++j) {
        size_t i = j * w / RB_AUDIO_FRAMES_PER_VIDEO_FRAME;
        float s = pAnalysis->rawAudio[i] * 0.5f + 0.5f;
        mins[i] = rbMinF(s, mins[i]);
        maxs[i] = rbMaxF(s, maxs[i]);
        if(mins[i] < 0.0f) {
            mins[i] = 0.0f;
        }
        if(maxs[i] > 1.0f) {
            maxs[i] = 1.0f;
        }
        avg += pAnalysis->rawAudio[i] / RB_AUDIO_FRAMES_PER_VIDEO_FRAME;
    }
    
    t2clear(pFrame, bgC);
    for(size_t i = 0; i < w; ++i) {
        float y0 = mins[i] * (oscopeH - 1);
        float y0c = ceilf(y0); // y0c = ceil
        float y0a = y0c - y0; // y0a = alpha
        float y1 = maxs[i] * (oscopeH - 1);
        float y1f = floorf(y1); // y1f = floor
        float y1a = y1 - y1f; // y1a = alpha
        
        if(y0a > 0.0f) {
            t2sett(pFrame, i, h - 1 - (int32_t)floorf(y0),
                cscalef(oscopeC, y0a));
        }
        for(int32_t y = (int32_t)y0c; y < (int32_t)y1f + 1; ++y) {
            t2sett(pFrame, i, h - 1 - y, oscopeC);
        }
        if(y1a > 0.0f) {
            t2sett(pFrame, i, h - 1 - (int32_t)ceilf(y1),
                cscalef(oscopeC, y1a));
        }
    }
    t2dtextf(pFrame, w * 0 / 8, h * 4 / 8, colori(255, 255, 255, 255),
        "%+6.3f", avg);
    {
        float lAgcMax = logf(g_rbConfiguration.agcMax);
        float lAgcMin = logf(g_rbConfiguration.agcMin);
        float lAgc = logf(pAnalysis->agcValue);
        t2rect(pFrame, w * 0 / 8, h * 0 / 8,
            roundf(((lAgc - lAgcMin) / (lAgcMax - lAgcMin)) * w * 8 / 8),
            h * 1 / 8, agcC);
    }
    t2rect(pFrame, w * 0 / 8, h * 1 / 8,
        rbMinI((int32_t)roundf(pAnalysis->bassEnergy * 0.5f * w * 4 / 8),
            w * 4 / 8), h * 1 / 8, bassC);
    t2rect(pFrame, w * 4 / 8, h * 1 / 8,
        rbMinI((int32_t)roundf(pAnalysis->trebleEnergy * 0.5f * w * 4 / 8),
            w * 4 / 8), h * 1 / 8, trebC);
    t2rect(pFrame, w * 0 / 8, h * 2 / 8,
        rbMinI((int32_t)roundf(pAnalysis->leftRightBalance * w * 4 / 8),
            w * 4 / 8), h * 1 / 8, lrBalC);
    if(pAnalysis->sourceOverdriven) {
        t2rect(pFrame, w * 1 / 8, h * 3 / 8, w * 2 / 8, h * 1 / 8, odC);
    }
    if(pAnalysis->sourceLargeDc) {
        t2rect(pFrame, w * 3 / 8, h * 3 / 8, w * 2 / 8, h * 1 / 8, ldcC);
    }
    if(pAnalysis->peakDetected) {
        t2rect(pFrame, w * 5 / 8, h * 3 / 8, w * 2 / 8, h * 1 / 8, pdC);
    }
}


void rbOverlayFrameControlsDebugInfo(RBAnalyzedAudio * pAnalysis,
    RBTexture2 * pFrame)
{
    size_t w = t2getw(pFrame);
    size_t h = t2geth(pFrame);
    RBColor const bgC = colori(0, 0, 0, 0);
    
    t2clear(pFrame, bgC);
    for(size_t i = 0; i < 5; ++i) {
        t2rect(pFrame, w * 0 / 8, h * i / 8,
            roundf((pAnalysis->controls.controllers[i] * 0.5f + 0.5f) *
                w * 4 / 8), h * 1 / 8, g_rbDebugColors[i]);
        t2rect(pFrame, w * 4 / 8, h * i / 8,
            roundf((pAnalysis->controls.controllers[i + 5] * 0.5f + 0.5f) *
                w * 4 / 8), h * 1 / 8, g_rbDebugColors[i + 5]);
    }
    for(size_t i = 0; i < 10; ++i) {
        if(pAnalysis->controls.triggers[i]) {
            t2rect(pFrame, w * (i % 4) * 2 / 8, h * (i / 4 + 5) * 1 / 8,
                w * 2 / 8, h * 1 / 8, g_rbDebugColors[i]);
        }
    }
}


void rbOverlayFramePerfMetricsDebugInfo(RBAnalyzedAudio * pAnalysis,
    RBTexture2 * pFrame)
{
    size_t w = t2getw(pFrame);
    size_t h = t2geth(pFrame);
    
    UNUSED(pAnalysis);
    
    if(w >= RB_DEBUG_CHAR_WIDTH * 8) {
        t2rect(pFrame, 0, 0, w, h, colori(0, 0, 0, 192));
        t2dtextf(pFrame, 0, h - RB_DEBUG_CHAR_HEIGHT * 2,
            colori(255, 128, 0, 255), "%6.3fms",
            g_rbAverageFrameComputeNs * 1e-6f);
        t2dtextf(pFrame, 0, h - RB_DEBUG_CHAR_HEIGHT, colori(255, 128, 0, 255),
            "%6.3fms", (float)g_rbLastFrameComputeNs * 1e-6f);
    }
    t2rect(pFrame, 0, h * 0 / 8,
        floorf((float)g_rbLastFrameComputeNs * 1e-9f *
            (float)RB_VIDEO_FRAME_RATE * 0.5f * w),
        h / 8, colori(128, 128, 128, 255));
    t2rect(pFrame, floorf((float)g_rbPeakFrameComputeNs * 1e-9f *
            (float)RB_VIDEO_FRAME_RATE * 0.5f * w), h * 0 / 8, 1, h * 1 / 8,
        colori(255, 0, 255, 255));
}


void rbOverlayFrameProjectionGridDebugInfo(RBAnalyzedAudio * pAnalysis,
    RBTexture2 * pFrame)
{
    int32_t w = t2getw(pFrame);
    int32_t h = t2geth(pFrame);
    int32_t animFrame =
        g_rbDebugDisplayFrameCounter * 10 / RB_VIDEO_FRAME_RATE;
    RBColor const bgC = colori(15, 15, 15, 0);
    RBColor const frameC = colori(127, 127, 127, 255);
    RBColor const gridC = colori(63, 15, 0, 255);
    RBColor const sweepC = colori(15, 63, 0, 0);
    
    UNUSED(pAnalysis);
    
    if(animFrame >= w + h) {
        animFrame = 0;
        g_rbDebugDisplayFrameCounter = 0;
    }
    
    t2clear(pFrame, bgC);
    for(int32_t i = 0; i < (h / 4) - 1; ++i) {
        t2rect(pFrame, 0, i * 4 + 3, w, 2, gridC);
    }
    for(int32_t i = 0; i < (w / 4) - 1; ++i) {
        t2rect(pFrame, i * 4 + 3, 0, 2, h, gridC);
    }
    
    t2dtextf(pFrame, (w - RB_DEBUG_CHAR_WIDTH * 1) / 2,
        (h - RB_DEBUG_CHAR_HEIGHT * 2) / 2, frameC, "^");
    t2dtextf(pFrame, (w - RB_DEBUG_CHAR_WIDTH * 1) / 2,
        (h - RB_DEBUG_CHAR_HEIGHT * 2) / 2, frameC, "|");
    t2dtextf(pFrame, (w - RB_DEBUG_CHAR_WIDTH * 2) / 2,
        (h - RB_DEBUG_CHAR_HEIGHT * 2) / 2 + RB_DEBUG_CHAR_HEIGHT * 1, frameC,
        "UP");
    
    t2rect(pFrame, 0, 0, w, 1, frameC);
    t2rect(pFrame, 0, h - 1, w, 1, frameC);
    t2rect(pFrame, 0, 0, 1, h, frameC);
    t2rect(pFrame, w - 1, 0, 1, h, frameC);
    
    if(animFrame < w) {
        t2rect(pFrame, animFrame, 0, 1, h, sweepC);
    }
    else {
        t2rect(pFrame, 0, animFrame - w, w, 1, sweepC);
    }
}


void rbOverlayRawDebugInfo(RBAnalyzedAudio * pAnalysis,
    RBRawLightFrame * pRawFrame)
{
    switch(pAnalysis->controls.debugDisplayMode) {
    case RBDM_OFF:
        break;
    case RBDM_AUDIO:
    case RBDM_CONTROLS:
    case RBDM_PERF_METRICS:
    case RBDM_PROJECTION_GRID:
        break;
    case RBDM_IDENTIFY_PIXELS:
        rbOverlayRawIdentifyPixelsDebugInfo(pAnalysis, pRawFrame);
        break;
    default:
        rbFatal("Invalid debug display mode??\n");
        break;
    }
    UNUSED(pRawFrame);
}


void rbOverlayRawIdentifyPixelsDebugInfo(RBAnalyzedAudio * pAnalysis,
    RBRawLightFrame * pRawFrame)
{
    size_t const lps = pRawFrame->numLightsPerString;
    uint32_t animFrame =
        g_rbDebugDisplayFrameCounter * 10 / RB_VIDEO_FRAME_RATE;
    
    UNUSED(pAnalysis);
    
    if(animFrame >= lps) {
        animFrame = 0;
        g_rbDebugDisplayFrameCounter = 0;
    }
    
    rbAssert(pRawFrame->numLightStrings < LENGTHOF(g_rbDebugColors));
    for(size_t j = 0; j < pRawFrame->numLightStrings; ++j) {
        for(size_t i = 0; i < lps; ++i) {
            size_t index = j * lps + i;
            if(i == animFrame) {
                pRawFrame->data[index] = colori(255, 255, 255, 255);
            }
            else {
                size_t const panelSize = RB_PANEL_WIDTH * RB_PANEL_HEIGHT;
                size_t panelCoord = i % panelSize;
                size_t panelNum = i / panelSize;
                if(panelCoord == panelSize - 1 || panelCoord < panelNum) {
                    pRawFrame->data[index] = cscalei(g_rbDebugColors[j],
                        (animFrame % 4) * 32 + 31);
                }
                else {
                    pRawFrame->data[index] = g_rbDebugColors[j];
                }
            }
        }
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
    
    rbChangeSubsystem(RBS_PARAMETER_GENERATION);
    rbParameterGenerationInitialize(&g_rbConfiguration);
    
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


