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


#define SL_AUDIO_CHANNELS 2
#define SL_AUDIO_SAMPLE_RATE 48000
#if defined SL_USE_TARGET_HARNESS
#define SL_VIDEO_FRAME_RATE 60
#else
#define SL_VIDEO_FRAME_RATE 60
#endif
#define SL_AUDIO_FRAMES_PER_VIDEO_FRAME ((SL_AUDIO_SAMPLE_RATE / SL_VIDEO_FRAME_RATE)*2)

#define SL_PANEL_WIDTH 8
#define SL_PANEL_HEIGHT 8

#define SL_NUM_LIGHTS (SL_PANEL_WIDTH * SL_PANEL_HEIGHT * 3)

#define SL_MAX_CONSECUTIVE_GENTLE_RESTART_NS (10 * 1000000000LLU)
#define SL_GENTLE_RESTART_DELAY_NS 500000000LLU

#define SL_E (2.71828182845905f)


typedef enum {
    SLLL_INFO,
    SLLL_WARNING,
    SLLL_ERROR,
    SLLL_COUNT
} SLLogLevel;


typedef enum {
    SLS_MAIN,
    SLS_CONFIGURATION,
    SLS_AUDIO_INPUT,
    SLS_AUDIO_ANALYSIS,
    SLS_LIGHT_GENERATION,
    SLS_LIGHT_OUTPUT,
    SLS_HOT_CONFIGURATION,
    SLS_COUNT
} SLSubsystem;


typedef enum {
    SLAIS_INVALID,
    SLAIS_ALSA,
    SLAIS_FILE,
} SLAudioInputSource;


#ifdef SL_USE_NEON
typedef uint16x4_t SLColor;
#else
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t x;
} SLColor;
#endif


typedef struct {
    SLColor c[5];
} SLPalette;


typedef int32_t SLCoord;


typedef int32_t SLTime;


typedef struct {
    SLTime time;
    SLTime period;
} SLTimer;


typedef struct {
    SLLogLevel logLevel;
    
    // Not read/written to config file, obviously. Comes from command line or
    // defaults
    char configPath[PATH_MAX];
    
    //
    SLAudioInputSource audioSource;
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
} SLConfiguration;


typedef struct {
    float audio[SL_AUDIO_FRAMES_PER_VIDEO_FRAME][SL_AUDIO_CHANNELS];
} SLRawAudio;


typedef struct {
    float bassEnergy;
    float midEnergy;
    float trebleEnergy;
    float totalEnergy;
    float leftRightBalance;
} SLAnalyzedAudio;


typedef struct {
    SLColor data[SL_PANEL_HEIGHT][SL_PANEL_WIDTH];
} SLPanel;    


typedef struct {
    SLPanel left;
    SLPanel right;
    SLPanel overhead;
} SLLightData;


// Tell the main program which subsystem is currently running (in this thread).
// The main subsystem will handle changing the current subsystem when it calls
// into subsystems, but this may be needed for inter-subsystem calls.
SLSubsystem slChangeSubsystem(SLSubsystem subsystem);

// In some exceptional cases we may need to reinitialize some subsystems
void slRequestGentleRestart(void);
void slRequestDelayedGentleRestart(void);

// If we've encountered an error condition that indicates an insane program
// state, use immediate restart to prevent any further execution and restart
// the whole process
void slRequestImmediateRestart(void);

// Returns true if we are in the middle of a subsystem sl*Initialize() due to
// a gentle restart; false if this is first-time init.
bool slIsRestarting(void);

static inline SLTime slTimeFromMs(int32_t ms)
{
    return ms;
}

SLTime slGetTime(void);

static inline SLTime slDiffTime(SLTime x, SLTime y)
{
    return x - y;
}

static inline SLTime slGetTimeSince(SLTime x)
{
    return slDiffTime(slGetTime(), x);
}

void slStartTimer(SLTimer * pTimer, SLTime period);

static inline void slStopTimer(SLTimer * pTimer)
{
    slStartTimer(pTimer, 0);
}

int32_t slGetTimerPeriods(SLTimer * pTimer);
int32_t slGetTimerPeriodsAndReset(SLTimer * pTimer);

static inline bool slTimerElapsed(SLTimer * pTimer)
{
    return !!slGetTimerPeriods(pTimer);
}

SLTime slGetTimeLeft(SLTimer * pTimer);

// Logging functions; these are for internal consumption only, please use the
// macros below (slInfo/slWarning/SlError...)
void slLog(SLLogLevel level, char const * sourceFile, int sourceLine,
    char const * format, ...);
bool slLogShouldLog(SLLogLevel level, char const * sourceFile,
    int sourceLine);
void slLogOutput(char const * format, ...);

// Reset stray NaN/+inf/-inf values; for algorithms that preserve state frame
// to frame it's easy to become "infected" with these values. This function
// can be used to strategically scrub them
static inline void slSanitizeFloat(float * pF, float saneValue)
{
    if(isinf(*pF) || isnan(*pF)) {
        *pF = saneValue;
    }
}

static inline void slSanitizeDouble(double * pF, double saneValue)
{
    if(isinf(*pF) || isnan(*pF)) {
        *pF = saneValue;
    }
}

static inline float slClampF(float f, float fMin, float fMax)
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
#define slAssert(assertExpr) do {} while(false)
#else
#define slAssert(assertExpr) \
    do { \
        if(!(assertExpr)) { \
            slLog(SLLL_ERROR, __FILE__, __LINE__, \
                "Assert fail (%s:%d): expected %s\n", \
                __FILE__, __LINE__, #assertExpr); \
            slRequestImmediateRestart(); \
        } \
    } while(false)
#endif

#define slVerify(verifyExpr) \
    do { \
        if(!(verifyExpr)) { \
            slLog(SLLL_ERROR, __FILE__, __LINE__, \
                "Fatal error (%s:%d): expected %s\n", __FILE__, __LINE__, \
                #verifyExpr); \
            slRequestImmediateRestart(); \
        } \
    } while(false)

#define slFatal(...) \
    do { \
        if(slLogShouldLog(SLLL_ERROR, __FILE__, __LINE__)) { \
            slLog(SLLL_ERROR, __FILE__, __LINE__, "Fatal error: "); \
            slLogOutput(__VA_ARGS__); \
        } \
        slRequestImmediateRestart(); \
    } while(false)

#ifdef NDEBUG
#define slDebugInfo(...) do {} while(false)
#else
#define slDebugInfo(...) slLog(SLLL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#endif
 
#define slInfo(...) slLog(SLLL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define slWarning(...) slLog(SLLL_WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define slError(...) slLog(SLLL_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#define slInfoEnabled() slLogShouldLog(SLLL_INFO, __FILE__, __LINE__)
#define slInfoWarningEnabled() \
    slLogShouldLog(SLLL_WARNING, __FILE__, __LINE__)
#define slInfoErrorEnabled() slLogShouldLog(SLLL_ERROR, __FILE__, __LINE__)


// Framework: coordinates all the other subsystems
void slInitialize(int argc, char * argv[]);
void slShutdown(void);
void slProcess(uint64_t nsSinceLastProcess);


// Implemented in the harness!
void slLogOutputV(char const * format, va_list va);
void slLightOutputInitialize(SLConfiguration const * config);
void slLightOutputShutdown(void);
void slLightOutputShowLights(SLLightData const * lights);


#endif // STAGE_LIGHTS_H_INCLUDED

