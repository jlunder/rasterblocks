
// Rebuild of https://github.com/jirislaby/collected_sources/tree/master/snd_fft
// TODO: Strip out graphics and sound code
// Pass fft data to next layer

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <ncurses.h>

#include <alsa/asoundlib.h>
#include <sndfile.h>
#include <complex.h>
#include <fftw3.h>
//sudo apt-get install libfftw3-dev libfftw3-doc
//sudo apt-get install libncurses5-dev

static SNDFILE *inFile;
static SF_INFO sfInfo;

static snd_pcm_t *alsaHandle;

static fftw_complex *fftOut;
static fftw_plan plan;
static int height, width;

static unsigned int fftLen;

static double *fftData;

static double maxLevel;

static void openWaveFile(const char* inFileName);
static void initPlot();
static void setupFftw();
static void destroyFftw();
static void playWav();
static void closeWaveFile();
static void averageChannels(float* audioBuffer, unsigned int audioLen);
static void computeFftw();
static void plotSpec();
static void soundOut(float* audioBuffer, unsigned int audioLen);

int main (int argc, char** argv)
{
	const char *inFileName = "fileIn.wav";

	openWaveFile(inFileName);
	initPlot();

	fftLen = sfInfo.samplerate / 100;

	if (width > fftLen / 2) {
		width = fftLen / 2;
	}

	setupFftw()
;
	playWav();

	destroyFftw();

	closeWaveFile();
}

// Strip this out
static void soundOut(float* audioBuffer, unsigned int audioLen)
{
	snd_pcm_sframes_t frames;

	frames = snd_pcm_writei(alsaHandle, audioBuffer, audioLen);
	if (frames < 0)
		frames = snd_pcm_recover(alsaHandle, frames, 0);
	if (frames < 0)
		printf("snd_pcm_writei failed");
}

// Strip this out
static void plotSpec()
{
	unsigned int realCount = fftLen/2;
	unsigned int sum = 0;
	unsigned int index = 0;
	unsigned int max = 0;
	unsigned int range =  realCount / width;
	unsigned int x = 0;

	erase();

	for (int i = 0; i < realCount; i++) {
		unsigned int sample = fftData[i] * height / maxLevel;
		if (sample > max) {
			max = sample;
		}
		sum += sample;
		if (index++ == range) {
			sum /= range;
			color_set(1, NULL);
			for (int j = height - max; j < height - sum; j++) {
				mvaddch(j, x, '|');
			}
			color_set(2, NULL);
			for (int j = height - sum; j < height; j++) {
				mvaddch(j, x, '*');
			}
			max = sum = index = 0;
			x++;
		}
	}
	move(0, 0);
	refresh();
}

static void computeFftw() 
{
	fftw_execute(plan);

	for (int i = 0; i < fftLen/2; i++) {
		fftData[i] = cabs(fftOut[i]);

		if (fftData[i] > maxLevel) {
			maxLevel = fftData[i];
		}
	}
}

static void averageChannels(float* audioBuffer, unsigned int audioLen)
{
	for (int i = 0; i < audioLen; i++) {
		fftData[i] = 0;
		for (int j = 0; j < sfInfo.channels; j++) {
			fftData[i] += audioBuffer[i*sfInfo.channels+j];
		}
		fftData[i] /= sfInfo.channels;
	}
}

static void playWav() 
{
	unsigned int channels = sfInfo.channels;
	float audioBuffer[channels*fftLen];

	int readCount = 0;
	int continueRead = 0;

	while ((readCount = sf_readf_float(inFile, audioBuffer, fftLen)) > 0) {
		continueRead = readCount != fftLen;
		if (!continueRead) {
			averageChannels(audioBuffer, fftLen);
			computeFftw();
			plotSpec();
		}
		soundOut(audioBuffer, fftLen);
	}

}

static void setupFftw()
{
	fftData = fftw_alloc_real(fftLen);
	if (!fftData) {
		printf("input alloc failed");
	}
	
	fftOut = fftw_alloc_complex(fftLen);
	if (!fftOut) {
		printf("output alloc failed");
	}

	for (int i = 0; i < fftLen; i++) {
		fftData[i] = 0;
		fftOut[i] = 0;
	}

	plan = fftw_plan_dft_r2c_1d(fftLen, fftData, fftOut, 0);

	if (!plan) {
		printf("plan failed");
	}

}

static void destroyFftw()
{
	fftw_destroy_plan(plan);
	fftw_free(fftOut);
	fftw_free(fftData);
}

static void initPlot()
{
	initscr();
	start_color();
	init_pair(1, COLOR_BLUE, COLOR_BLACK);
	init_pair(2, COLOR_WHITE, COLOR_BLACK);
	getmaxyx(stdscr, height, width);
}

static void openWaveFile(const char* inFileName)
{
	inFile = sf_open(inFileName, SFM_READ, &sfInfo);

	if (!inFile) {
		printf("Not able to open input file %s.\n", inFileName);
		return;
	}

	int error = 0;
	error = snd_pcm_open(&alsaHandle, "default", SND_PCM_STREAM_PLAYBACK, 0);

	if (error < 0) {
		printf("alsa open failed");
	}

	error =  snd_pcm_set_params(alsaHandle, SND_PCM_FORMAT_FLOAT,
						   SND_PCM_ACCESS_RW_INTERLEAVED, sfInfo.channels,
						   sfInfo.samplerate, 1, 500000);
	if (error < 0) {
		printf("alsa set params failed");
	}
}

static void closeWaveFile()
{
	endwin();
	snd_pcm_close(alsaHandle);
	sf_close(inFile);
}
