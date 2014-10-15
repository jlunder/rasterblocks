#ifndef STAGE_LIGHTS_H_INCLUDED
#define STAGE_LIGHTS_H_INCLUDED


#include <limits.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


#define UNUSED(x) (void)(x)


#define SL_NUM_LIGHTS 10


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
} SLRawAudio;

typedef struct {
} SLAnalyzedAudio;

typedef struct {
    SLColor data[SL_NUM_LIGHTS];
} SLLightData;

typedef enum {
    SLLL_INFO,
    SLLL_WARNING,
    SLLL_ERROR,
} SLLogLevel;

typedef enum {
    SLS_MAIN,
    SLS_CONFIGURATION,
    SLS_AUDIO_INPUT,
    SLS_AUDIO_ANALYSIS,
    SLS_LIGHT_GENERATION,
    SLS_LIGHT_OUTPUT,
    SLS_HOT_CONFIGURATION,
} SLSubsystem;


extern SLColor slHarnessLights[SL_NUM_LIGHTS];


// Tell the main program which subsystem is currently running (in this thread)
SLSubsystem slChangeSubsystem(SLSubsystem subsystem);
void slRequestGentleRestart(void);
void slRequestImmediateRestart(void);
#define slAssert(x)
void slLog(SLLogLevel level, char const * format, ...);
#define slInfo(format, ...) slLog(SLLL_INFO, __VA_ARGS__)
#define slWarning(format, ...) slLog(SLLL_WARNING, __VA_ARGS__)
#define slError(format, ...) slLog(SLLL_ERROR, __VA_ARGS__)


// Main subsystem: coordinates all the other subsystems
void slInitialize(int argc, char * argv[]);
void slShutdown(void);
void slProcess(uint64_t nsSinceLastProcess);


// Configurations subsystem
void slConfigurationSetDefaults(SLConfiguration * config);
void slConfigurationLoad(SLConfiguration * config);
void slConfigurationSave(SLConfiguration const * config);


// Audio input subsystem
void slAudioInputInitialize(SLConfiguration const * config);
void slAudioInputShutdown(void);
void slAudioInputBlockingRead(SLRawAudio * audio);


// Audio analysis subsystem
void slAudioAnalysisInitialize(SLConfiguration const * config);
void slAudioAnalysisShutdown(void);
void slAudioAnalysisAnalyze(SLRawAudio const * audio,
    SLAnalyzedAudio * analysis);


// Light generation subsystem
void slLightGenerationInitialize(SLConfiguration const * config);
void slLightGenerationShutdown(void);
void slLightGenerationGenerate(SLAnalyzedAudio const * analysis,
    SLLightData * lights);


// Light output subsystem
void slLightOutputInitialize(SLConfiguration const * config);
void slLightOutputShutdown(void);
void slLightOutputShowLights(SLLightData const * lights);


// Hot configuration subsystem
void slHotConfigurationInitialize(SLConfiguration const * config);
void slHotConfigurationShutdown(void);
void slHotConfigurationProcessAndUpdateConfiguration(SLConfiguration * config,
    bool * configurationModified);


#endif // STAGE_LIGHTS_H_INCLUDED

