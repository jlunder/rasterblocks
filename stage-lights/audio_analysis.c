#include "audio_analysis.h"

#include "analysis/levels.h"

void slAudioAnalysisInitialize(SLConfiguration const * config)
{
    UNUSED(config);
}


void slAudioAnalysisShutdown(void)
{
}


void slAudioAnalysisAnalyze(SLRawAudio const * audio, SLAnalyzedAudio * analysis)
{
    //UNUSED(audio);
    //UNUSED(analysis);

	levels_to_bass_analyze(audio,analysis,SL_AUDIO_FRAMES_PER_VIDEO_FRAME,2);
}


