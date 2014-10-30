#include <fftw3.h>
#include <math.h>
#include <stdlib.h> 

#include "audio_analysis.h"

#include "analysis/levels.h"

static float *fftData;
static int fftLen;
static fftwf_plan fftPlan;
static fftwf_complex *fftOutputData;
static int fftMax;


static void sumChannelsIntoFFTIn(SLRawAudio const * audio, int audioLen);
static void computeFFT();

void slAudioAnalysisInitialize(SLConfiguration const * config)
{
    UNUSED(config);

    fftLen = SL_AUDIO_FRAMES_PER_VIDEO_FRAME;

    fftData = fftwf_alloc_real(fftLen);
    if (!fftData) {
    	printf("fftw input alloc failed");
    }

    fftOutputData = fftwf_alloc_complex(fftLen);
    if (!fftOutputData) {
    	printf("fftw output alloc failed");
    }


    fftPlan = fftwf_plan_dft_r2c_1d(fftLen, fftData, fftOutputData, 0);
    if (!fftPlan) {
    	printf("fftw plan failed");
    }
}


void slAudioAnalysisShutdown(void)
{
	fftwf_destroy_plan(fftPlan);
	fftwf_free(fftOutputData);
	fftwf_free(fftData);
}


void slAudioAnalysisAnalyze(SLRawAudio const * audio, SLAnalyzedAudio * analysis)
{
	sumChannelsIntoFFTIn(audio, SL_AUDIO_FRAMES_PER_VIDEO_FRAME);
	computeFFT();

    //UNUSED(audio);
    //UNUSED(analysis);

	slAnalysisBasicLevels(audio,analysis,SL_AUDIO_FRAMES_PER_VIDEO_FRAME,2);
}

static void sumChannelsIntoFFTIn(SLRawAudio const * audio, int audioLen)
{
	for (int i = 0; i < audioLen; i++) {
		fftData[i] = 0;
		// TODO: get channels
		for (int j = 0; j < 2; j++) {
			fftData[i] += audio->audio[i][j];
		}
		fftData[i] /= 2;
	}
}

static void computeFFT()
{
	fftMax = 0;
	fftwf_execute(fftPlan);

	for (int i = 0; i < fftLen/2; i++) {
		fftData[i] = abs(fftOutputData[0][i]);

		if (fftData[i] > fftMax) {
			fftMax = fftData[i];
		}
	}

	// Normalize values
	for (int i = 0; i < fftLen/2; i++) {
		fftData[i] /= fftMax;
	}
}


