#ifndef AUDIO_ANALYSIS_H_INCLUDED
#define AUDIO_ANALYSIS_H_INCLUDED


#include "rasterblocks.h"


// Audio analysis subsystem
void slAudioAnalysisInitialize(SLConfiguration const * config);
void slAudioAnalysisShutdown(void);
void slAudioAnalysisAnalyze(SLRawAudio const * audio,
    SLAnalyzedAudio * analysis);


#endif

