#ifndef AUDIO_ANALYSIS_H_INCLUDED
#define AUDIO_ANALYSIS_H_INCLUDED


#include "rasterblocks.h"


// Audio analysis subsystem
void rbAudioAnalysisInitialize(RBConfiguration const * config);
void rbAudioAnalysisShutdown(void);
void rbAudioAnalysisAnalyze(RBRawAudio const * audio,
    RBAnalyzedAudio * analysis);


#endif

