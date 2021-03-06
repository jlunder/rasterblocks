#ifndef RASTERBLOCKS_H_INCLUDED
#define RASTERBLOCKS_H_INCLUDED


#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/lualib.h"


#define UNUSED(x) (void)(x)
#define LENGTHOF(x) (sizeof (x) / sizeof (*(x)))


#define RB_AUDIO_CHANNELS 2
#define RB_AUDIO_SAMPLE_RATE 48000
#if defined RB_USE_TARGET_HARNESS
#define RB_VIDEO_FRAME_RATE 60
#else
#define RB_VIDEO_FRAME_RATE 60
#endif
#define RB_VIDEO_FRAME_TIME (1.0f / RB_VIDEO_FRAME_RATE)
#define RB_AUDIO_FRAMES_PER_VIDEO_FRAME \
    (RB_AUDIO_SAMPLE_RATE / RB_VIDEO_FRAME_RATE)
#define RB_MIDI_BAUD 30000
#define RB_MIDI_MAX_CHARS_PER_SECOND (RB_MIDI_BAUD / 10 + 1)
#define RB_MIDI_MAX_CHARS_PER_VIDEO_FRAME \
    ((RB_MIDI_BAUD + RB_VIDEO_FRAME_RATE * 10 - 1) / \
        (RB_VIDEO_FRAME_RATE * 10) + 1)

#define RB_CHLADNI_SIZE 64

#define RB_NUM_CONTROLLERS 10
#define RB_NUM_TRIGGERS 10
#define RB_NUM_PARAMETERS 40
#define RB_PARAMETER_NONE 0x10000

#define RB_MAX_LIGHTS 2048
#define RB_MAX_LIGHT_STRINGS 8

#define RB_PANEL_WIDTH 8
#define RB_PANEL_HEIGHT 8

#define RB_MAX_CONSECUTIVE_GENTLE_RESTART_NS (10 * 1000000000LLU)
#define RB_GENTLE_RESTART_DELAY_NS 500000000LLU

#define RB_E (2.71828182845905f)
#define RB_PI (3.14159265358979f)
#define RB_PI_D (3.14159265358979323846)
#define RB_SQRT_2 (1.4142135623730950488f)

#define RB_AUDIO_OVERDRIVE_THRESHOLD (0.75f)
#define RB_AUDIO_LARGE_DC_THRESHOLD (0.25f)


typedef enum {
    RBLL_INFO,
    RBLL_WARNING,
    RBLL_ERROR,
    RBLL_COUNT
} RBLogLevel;


typedef enum {
    RBS_MAIN,
    RBS_CONFIGURATION,
    RBS_CONTROL_INPUT,
    RBS_AUDIO_INPUT,
    RBS_AUDIO_ANALYSIS,
    RBS_PARAMETER_GENERATION,
    RBS_LIGHT_GENERATION,
    RBS_LIGHT_OUTPUT,
    RBS_HOT_CONFIGURATION,
    RBS_LUA,
    RBS_COUNT
} RBSubsystem;


typedef enum {
    RBAI_INVALID,
    RBAI_TEST,
    RBAI_FILE,
    RBAI_ALSA,
    RBAI_OPENAL,
    RBAI_PRUSS,
} RBAudioInput;


typedef enum {
    RBCI_INVALID,
    RBCI_NONE,
    RBCI_TEST,
    RBCI_HARNESS,
    RBCI_BBB_UART4_MIDI,
    RBCI_PRUSS_MIDI,
} RBControlInput;


typedef enum {
    RBLO_INVALID,
    RBLO_OPENGL,
    RBLO_PIXELPUSHER,
    RBLO_SPIDEV,
    RBLO_PRUSS,
} RBLightOutput;


typedef enum {
    RBDM_OFF = 0, // Must be entry 0, so that rbZero(&rbControls...) will work
    RBDM_AUDIO,
    RBDM_CONTROLS,
    RBDM_PERF_METRICS,
    RBDM_PROJECTION_GRID,
    RBDM_IDENTIFY_PIXELS,
    RBDM_COUNT,
} RBDebugDisplayMode;


#ifdef RB_USE_NEON
typedef uint16x4_t RBColor;
#else
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} RBColor;
#endif


typedef struct {
    float x, y;
} RBVector2;


typedef struct {
    float x, y, z, w;
} RBVector4;


typedef struct {
    float m00, m01, m10, m11;
} RBMatrix2;


typedef struct {
    float
        m00, m01, m02, m03,
        m10, m11, m12, m13,
        m20, m21, m22, m23,
        m30, m31, m32, m33;
} RBMatrix4;


typedef int32_t RBCoord;


typedef int32_t RBTime;


typedef struct {
    RBTime time;
    RBTime period;
} RBTimer;


typedef struct {
    RBLogLevel logLevel;
    
    // Not read/written to config file, obviously. Comes from command line or
    // defaults
    char configPath[PATH_MAX];
    
    char luaPath[PATH_MAX];
    
    RBAudioInput audioInput;
    char audioInputParam[PATH_MAX];
    
    RBControlInput controlInput;
    
    RBLightOutput lightOutput;
    char lightOutputParam[PATH_MAX];
    
    float lowCutoff;
    float hiCutoff;
    
    float agcMax;
    float agcMin;
    float agcStrength;
    
    float brightness;
    int32_t projectionWidth;
    int32_t projectionHeight;
    size_t numLightStrings;
    size_t numLightsPerString;
    RBVector2 lightPositions[RB_MAX_LIGHTS];
    
    int32_t mode;
} RBConfiguration;


typedef struct {
    RBVector2 position;
    RBVector2 uInc;
    RBVector2 vInc;
} RBPanelConfig;


typedef struct {
    uint64_t frameNum;
    bool overdriven;
    bool largeDc;
    float audio[RB_AUDIO_FRAMES_PER_VIDEO_FRAME][RB_AUDIO_CHANNELS];
} RBRawAudio;


typedef struct {
    float controllers[RB_NUM_CONTROLLERS];
    bool triggers[RB_NUM_TRIGGERS];
    bool debugDisplayReset;
    RBDebugDisplayMode debugDisplayMode;
} RBControls;


typedef struct {
    uint64_t frameNum;
    RBTime time;
    
    RBControls controls;
    
    float rawAudio[RB_AUDIO_FRAMES_PER_VIDEO_FRAME];
    float bassAudio[RB_AUDIO_FRAMES_PER_VIDEO_FRAME];
    float trebleAudio[RB_AUDIO_FRAMES_PER_VIDEO_FRAME];
    
    float bassEnergy;
    float midEnergy;
    float trebleEnergy;
    float totalEnergy;
    float leftRightBalance;
    
    float agcValue;
    
    bool sourceOverdriven;
    bool sourceLargeDc;
    bool peakDetected;
} RBAnalyzedAudio;


typedef struct {
    float parameters[RB_NUM_PARAMETERS];
} RBParameters;


typedef struct {
    uint64_t frameNum;
    size_t numLightStrings;
    size_t numLightsPerString;
    RBColor data[RB_MAX_LIGHTS];
} RBRawLightFrame;


#define rbMemoryBarrier() asm volatile("": : :"memory")


#ifdef NDEBUG
#define rbAssert(assertExpr) do {} while(false)
#else
#define rbAssert(assertExpr) \
    do { \
        if(!(assertExpr)) { \
            rbLog(RBLL_ERROR, __FILE__, __LINE__, \
                "Assert fail (%s:%d): expected %s\n", \
                __FILE__, __LINE__, #assertExpr); \
            rbRequestImmediateRestart(); \
        } \
    } while(false)
#endif

#define rbVerify(verifyExpr) \
    do { \
        if(!(verifyExpr)) { \
            rbLog(RBLL_ERROR, __FILE__, __LINE__, \
                "Fatal error (%s:%d): expected %s\n", __FILE__, __LINE__, \
                #verifyExpr); \
            rbRequestImmediateRestart(); \
        } \
    } while(false)

#define rbFatal(...) \
    do { \
        if(rbLogShouldLog(RBLL_ERROR, __FILE__, __LINE__)) { \
            rbLog(RBLL_ERROR, __FILE__, __LINE__, "Fatal error: "); \
            rbLogOutput(__VA_ARGS__); \
        } \
        rbRequestImmediateRestart(); \
    } while(false)

#ifdef NDEBUG
#define rbDebugInfo(...) do {} while(false)
#else
#define rbDebugInfo(...) rbLog(RBLL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#endif
 
#define rbInfo(...) rbLog(RBLL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define rbWarning(...) rbLog(RBLL_WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define rbError(...) rbLog(RBLL_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#define rbInfoEnabled() rbLogShouldLog(RBLL_INFO, __FILE__, __LINE__)
#define rbWarningEnabled() rbLogShouldLog(RBLL_WARNING, __FILE__, __LINE__)
#define rbErrorEnabled() rbLogShouldLog(RBLL_ERROR, __FILE__, __LINE__)


// Tell the main program which subsystem is currently running (in this thread).
// The main subsystem will handle changing the current subsystem when it calls
// into subsystems, but this may be needed for inter-subsystem calls.
RBSubsystem rbChangeSubsystem(RBSubsystem subsystem);

// In some exceptional cases we may need to reinitialize some subsystems
void rbRequestGentleRestart(void);
void rbRequestDelayedGentleRestart(void);

// If we've encountered an error condition that indicates an insane program
// state, use immediate restart to prevent any further execution and restart
// the whole process
void rbRequestImmediateRestart(void);

// If we suspect the hardware is in an insane state that can't be resolved
// within the program itself, use RequestHardwareReset to reset the whole
// system
void rbRequestHardwareReset(void);

// Returns true if we are in the middle of a subsystem rb*Initialize() due to
// a gentle restart; false if this is first-time init.
bool rbIsRestarting(void);

RBConfiguration const * rbGetConfiguration(void);
void rbComputeLightPositionsFromPanelList(RBVector2 * pLightPositions,
    size_t numLightPositions, RBPanelConfig const * pPanelConfigs,
    size_t numPanels);

static inline RBTime rbTimeFromMs(int32_t ms)
{
    return ms;
}

static inline int32_t rbMsFromTime(RBTime time)
{
    return time;
}

// rbGetTime() returns the *frame* time; it's invariant during computation of
// a frame. This is usually handy but if you are trying to time something
// (say, to measure performance), you need to use rbGetRealTime().
RBTime rbGetTime(void);

// This returns the time since the last frame, in seconds.
float rbGetDeltaTimeSeconds(void);

// rbGetRealTime() reads the clock at the exact point it is called. You should
// not use it for timing animations; rbGetTime() is better for that.
RBTime rbGetRealTime(void);
uint64_t rbGetRealTimeNs(void);

static inline RBTime rbDiffTime(RBTime x, RBTime y)
{
    return x - y;
}

static inline RBTime rbGetTimeSince(RBTime x)
{
    return rbDiffTime(rbGetTime(), x);
}

void rbSleep(RBTime x);

void rbStartTimer(RBTimer * pTimer, RBTime period);

static inline void rbStopTimer(RBTimer * pTimer)
{
    rbStartTimer(pTimer, 0);
}

static inline RBTime rbGetTimerPeriod(RBTimer * pTimer) {
    return pTimer->period;
}

int32_t rbGetTimerPeriods(RBTimer * pTimer);
int32_t rbGetTimerPeriodsAndReset(RBTimer * pTimer);

static inline bool rbTimerElapsed(RBTimer * pTimer)
{
    return !!rbGetTimerPeriods(pTimer);
}

RBTime rbGetTimeLeft(RBTimer * pTimer);

// Logging functions; these are for internal consumption only, please use the
// macros below (rbInfo/rbWarning/SlError...)
void rbLog(RBLogLevel level, char const * sourceFile, int sourceLine,
    char const * format, ...);
bool rbLogShouldLog(RBLogLevel level, char const * sourceFile,
    int sourceLine);
void rbLogOutput(char const * format, ...);


static inline void rbZero(void * p, size_t size)
{
    memset(p, 0, size);
}

static inline int rbStricmp(char const * pa, char const * pb)
{
    for(; ; ++pa, ++pb) {
        char ca = tolower(*pa);
        char cb = tolower(*pb);
        
        if(ca == cb) {
            if(ca != '\0') {
                continue;
            }
            else {
                return 0;
            }
        }
        if(ca < cb) {
            return -1;
        }
        else {
            return 1;
        }
    }
}

static inline void rbStrlcpy(char * dest, char const * src, size_t destSize)
{
    size_t srcSize = strlen(src);
    
    rbVerify(srcSize < destSize);
    memcpy(dest, src, srcSize + 1);
}


// Reset stray NaN/+inf/-inf values; for algorithms that preserve state frame
// to frame it's easy to become "infected" with these values. This function
// can be used to strategically scrub them
static inline void rbSanitizeFloat(float * pF, float saneValue)
{
    if(isinf(*pF) || isnan(*pF)) {
        *pF = saneValue;
    }
}

static inline void rbSanitizeDouble(double * pF, double saneValue)
{
    if(isinf(*pF) || isnan(*pF)) {
        *pF = saneValue;
    }
}

static inline int32_t rbClampI(int32_t i, int32_t iMin, int32_t iMax)
{
    if(i < iMin) {
        return iMin;
    }
    else if(i > iMax) {
        return iMax;
    }
    else {
        return i;
    }
}

static inline int32_t rbMinI(int32_t a, int32_t b)
{
    return a < b ? a : b;
}

static inline int32_t rbMaxI(int32_t a, int32_t b)
{
    return a > b ? a : b;
}

static inline float rbClampF(float f, float fMin, float fMax)
{
    // This is structured carefully so that NaN will clamp as fMin.
    // If you are modifying it, also consider +inf/-inf.
    if(f >= fMax) {
        return fMax;
    }
    if(f >= fMin) {
        return f;
    }
    return fMin;
}

static inline float rbMinF(float a, float b)
{
    return a < b ? a : b;
}

static inline float rbMaxF(float a, float b)
{
    return a > b ? a : b;
}

static inline uint32_t rbRandomI(uint32_t range)
{
    return ((rand() << 15) ^ rand()) % range;
}

static inline float rbRandomF(void)
{
    static float const randMul = 1.0f / ((float)RAND_MAX + 1.0f);
    return rand() * randMul;
}

static inline float rbParameterGetF(RBParameters const * pParameters,
    size_t index, float defaultValue)
{
    if(index <= RB_NUM_PARAMETERS - 1) {
        return pParameters->parameters[index];
    }
    else {
        return defaultValue;
    }
}


// Framework: coordinates all the other subsystems
void rbInitialize(int argc, char * argv[]);
void rbShutdown(void);
void rbProcess(void);


// Implemented in the harness!
void rbLogOutputV(char const * format, va_list va);


#endif // RASTERBLOCKS_H_INCLUDED

