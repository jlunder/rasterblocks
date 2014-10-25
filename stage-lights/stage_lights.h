#ifndef STAGE_LIGHTS_H_INCLUDED
#define STAGE_LIGHTS_H_INCLUDED


#include <limits.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


#define UNUSED(x) (void)(x)


#define SL_AUDIO_SAMPLE_RATE 48000
#define SL_VIDEO_FRAME_RATE 100
#define SL_AUDIO_FRAMES_PER_VIDEO_FRAME (SL_AUDIO_SAMPLE_RATE / SL_VIDEO_FRAME_RATE)

#define SL_NUM_LIGHTS 10

#define SL_MAX_CONSECUTIVE_GENTLE_RESTARTS SL_VIDEO_FRAME_RATE


typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t x;
} SLColor;


typedef enum {
    SLAIS_ALSA,
    SLAIS_FILE,
} SLAudioInputSource;


typedef struct {
    // Not read/written to config file, obviously. Comes from command line or
    // defaults
    char configPath[PATH_MAX];
    
    //
    SLAudioInputSource audioSource;
    char audioSourceParam[PATH_MAX];
    
    // analysis tweaks?
    // output mode?
    // output device path?
    // hot config port/etc.?
} SLConfiguration;


typedef struct {
    float audio[SL_AUDIO_FRAMES_PER_VIDEO_FRAME][2];
} SLRawAudio;


typedef struct {
    float bassEnergy;
    float midEnergy;
    float trebleEnergy;
    float totalEnergy;
    float leftRightBalance;
} SLAnalyzedAudio;


typedef struct {
    SLColor lights[SL_NUM_LIGHTS];
} SLLightData;


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


// Tell the main program which subsystem is currently running (in this thread).
// The main subsystem will handle changing the current subsystem when it calls
// into subsystems, but this may be needed for inter-subsystem calls.
SLSubsystem slChangeSubsystem(SLSubsystem subsystem);

// In some exceptional cases we may need to reinitialize some subsystems
void slRequestGentleRestart(void);

// If we've encountered an error condition that indicates an insane program
// state, use immediate restart to prevent any further execution and restart
// the whole process
void slRequestImmediateRestart(void);

void slLog(SLLogLevel level, char const * sourceFile, int sourceLine,
    char const * format, ...);
bool slLogShouldLog(SLLogLevel level, char const * sourceFile,
    int sourceLine);
void slLogOutput(char const * format, ...);

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

