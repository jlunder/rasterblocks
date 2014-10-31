
#include <stdlib.h> 

#include "audio_analysis.h"
#include "fft.h"
#include "filters.h"

#include "analysis/levels.h"

void slAudioAnalysisInitialize(SLConfiguration const * config)
{
    UNUSED(config);
    fftInitialize();
}


void slAudioAnalysisShutdown(void)
{
	fftDestroy();
}


void slAudioAnalysisAnalyze(SLRawAudio const * audio, SLAnalyzedAudio * analysis)
{
    //UNUSED(audio);
    //UNUSED(analysis);
    fftProcess(audio);
    slAnalsisFilterLevels(audio, analysis, SL_AUDIO_FRAMES_PER_VIDEO_FRAME, 2);
	slAnalysisBasicLevels(audio,analysis,SL_AUDIO_FRAMES_PER_VIDEO_FRAME,2);
}



