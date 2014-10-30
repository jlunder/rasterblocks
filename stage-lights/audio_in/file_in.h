
#include <sndfile.h>
#include <alsa/asoundlib.h>
#include <stdio.h>

static snd_pcm_t *alsaHandle;
static void open_alsa(SF_INFO sfinfo);
static void play_audio(float* audioBuffer, unsigned int audioLen);


SNDFILE* g_snd_file;

SNDFILE* slSndFileOpen(char *file_name) {
	SF_INFO  sfinfo;

	if ((g_snd_file = sf_open(file_name, SFM_READ, &sfinfo)) == NULL) {
		printf("Not able to open input file %s.\n", file_name);
		return NULL;
	}

	open_alsa(sfinfo);
	return g_snd_file;

}

void slSndFileClose() {
	sf_close(g_snd_file);
}


static void slSndFileReadLooping(SLRawAudio* audio_buf, int num_frames, int channels)
{
	float buffer[channels * num_frames];
	int readCount = sf_readf_float(g_snd_file, buffer, num_frames);
	play_audio(buffer, readCount);
	for (int frame = 0; frame < num_frames; frame++) {
		for (int channel = 0; channel < channels; channel++) {
			audio_buf->audio[frame][channel] = buffer[frame*channels+channel];
		}
	}
	if(readCount==0)
		sf_seek(g_snd_file,0,SEEK_SET);

	//printf("read: %i\n",readCount);

	/* play the audio here */

}

static void open_alsa(SF_INFO sfinfo)
{
	int error = 0;
	error = snd_pcm_open(&alsaHandle, "default", SND_PCM_STREAM_PLAYBACK, 0);

	if (error < 0) {
		printf("alsa open failed");
	}

	error =  snd_pcm_set_params(alsaHandle, SND_PCM_FORMAT_FLOAT,
						   SND_PCM_ACCESS_RW_INTERLEAVED, sfinfo.channels,
						   sfinfo.samplerate, 1, 500000);
	if (error < 0) {
		printf("alsa set params failed");
	}
}

static void play_audio(float* audioBuffer, unsigned int audioLen)
{
	snd_pcm_sframes_t frames;

	frames = snd_pcm_writei(alsaHandle, audioBuffer, audioLen);
	if (frames < 0) {
		frames = snd_pcm_recover(alsaHandle, frames, 0);
	}
	if (frames < 0) {
		printf("snd_pcm_writei failed");
	}
}
