#ifndef STAGE_LIGHTS_H_INCLUDED
#define STAGE_LIGHTS_H_INCLUDED


#include <limits.h>

#include <math.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define UNUSED(x) (void)(x)
#define LENGTHOF(x) (sizeof (x) / sizeof (*(x)))


#define RB_AUDIO_CHANNELS 2
#define RB_AUDIO_SAMPLE_RATE 48000
#if defined RB_USE_TARGET_HARNESS
#define RB_VIDEO_FRAME_RATE 50
#else
#define RB_VIDEO_FRAME_RATE 50
#endif
#define RB_AUDIO_FRAMES_PER_VIDEO_FRAME ((RB_AUDIO_SAMPLE_RATE / RB_VIDEO_FRAME_RATE)*2)

#define RB_PANEL_WIDTH 8
#define RB_PANEL_HEIGHT 8
#define RB_NUM_PANELS 6
#define RB_NUM_LIGHTS (RB_PANEL_WIDTH * RB_PANEL_HEIGHT * RB_NUM_PANELS)

#define RB_PROJECTION_WIDTH (RB_PANEL_WIDTH * 2)
#define RB_PROJECTION_HEIGHT (RB_PANEL_HEIGHT * 6)

#define RB_MAX_CONSECUTIVE_GENTLE_RESTART_NS (10 * 1000000000LLU)
#define RB_GENTLE_RESTART_DELAY_NS 500000000LLU

#define RB_E (2.71828182845905f)


typedef enum {
    RBLL_INFO,
    RBLL_WARNING,
    RBLL_ERROR,
    RBLL_COUNT
} RBLogLevel;


typedef enum {
    RBS_MAIN,
    RBS_CONFIGURATION,
    RBS_AUDIO_INPUT,
    RBS_AUDIO_ANALYSIS,
    RBS_LIGHT_GENERATION,
    RBS_LIGHT_OUTPUT,
    RBS_HOT_CONFIGURATION,
    RBS_COUNT
} RBSubsystem;


typedef enum {
    RBAIS_INVALID,
    RBAIS_DEVICE,
    RBAIS_FILE,
} RBAudioInputSource;


#ifdef RB_USE_NEON
typedef uint16x4_t RBColor;
#else
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t x;
} RBColor;
#endif


typedef struct {
    RBColor c[5];
} RBPalette;


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
    
    //
    RBAudioInputSource audioSource;
    char audioSourceParam[PATH_MAX];
    
    bool monitorAudio;

    float lowCutoff;
    float hiCutoff;

    float agcMax;
    float agcMin;
    float agcStrength;
    // analysis tweaks?
    // output mode?
    // output device path?
    // hot config port/etc.?
    
    float brightness;
} RBConfiguration;


typedef struct {
    float audio[RB_AUDIO_FRAMES_PER_VIDEO_FRAME][RB_AUDIO_CHANNELS];
} RBRawAudio;


typedef struct {
    float bassEnergy;
    float midEnergy;
    float trebleEnergy;
    float totalEnergy;
    float leftRightBalance;
} RBAnalyzedAudio;


typedef struct {
    RBVector2 position;
    RBVector2 orientation;
} RBPanelConfig;


typedef struct {
    RBColor proj[RB_PROJECTION_HEIGHT][RB_PROJECTION_WIDTH];
} RBProjectionFrame;


typedef struct {
    RBColor data[RB_NUM_PANELS][RB_PANEL_HEIGHT][RB_PANEL_WIDTH];
} RBRawLightFrame;


RBPanelConfig const g_rbPanelConfigs[RB_NUM_PANELS];


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

// Returns true if we are in the middle of a subsystem rb*Initialize() due to
// a gentle restart; false if this is first-time init.
bool rbIsRestarting(void);

static inline RBTime rbTimeFromMs(int32_t ms)
{
    return ms;
}

RBTime rbGetTime(void);

static inline RBTime rbDiffTime(RBTime x, RBTime y)
{
    return x - y;
}

static inline RBTime rbGetTimeSince(RBTime x)
{
    return rbDiffTime(rbGetTime(), x);
}

void rbStartTimer(RBTimer * pTimer, RBTime period);

static inline void rbStopTimer(RBTimer * pTimer)
{
    rbStartTimer(pTimer, 0);
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
#define rbInfoWarningEnabled() \
    rbLogShouldLog(RBLL_WARNING, __FILE__, __LINE__)
#define rbInfoErrorEnabled() rbLogShouldLog(RBLL_ERROR, __FILE__, __LINE__)


// Framework: coordinates all the other subsystems
void rbInitialize(int argc, char * argv[]);
void rbShutdown(void);
void rbProcess(uint64_t nsSinceLastProcess);


// Implemented in the harness!
void rbLogOutputV(char const * format, va_list va);
void rbLightOutputInitialize(RBConfiguration const * pConfig);
void rbLightOutputShutdown(void);
void rbLightOutputShowLights(RBRawLightFrame const * pFrame);


#endif // STAGE_LIGHTS_H_INCLUDED

