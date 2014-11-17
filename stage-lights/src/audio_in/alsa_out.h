#include <alsa/asoundlib.h>


static snd_pcm_t *playback_handle;

float* buffer;
int g_num_frames;
int g_channels;

static void slAlsaPlaybackInit(int num_frames, int channels)
{
	int error = 0;
	error = snd_pcm_open(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);

	if (error < 0) {
		slError("Alsa playback open failed\n");
	}

	error =  snd_pcm_set_params(playback_handle, SND_PCM_FORMAT_FLOAT,
						   SND_PCM_ACCESS_RW_INTERLEAVED, channels,
						   SL_AUDIO_SAMPLE_RATE, 1, 500000);
	if (error < 0) {
		slError("Alsa playback set params failed\n");
	}

    	buffer = malloc(num_frames*channels*sizeof(*buffer));
   	memset(buffer, 0, num_frames*channels*sizeof(*buffer));
}

void slAlsaPlaybackClose() {
    slInfo("Closing alsa playback device\n");
    snd_pcm_close(playback_handle);
    free(buffer);
}

void slAlsaPlayback(SLRawAudio* audio_buf, int num_frames, int channels) {

	for (int frame = 0; frame < num_frames; frame++) {
		for (int channel = 0; channel < channels; channel++) {
			buffer[frame*channels+channel] = audio_buf->audio[frame][channel];
		}
	}

	snd_pcm_sframes_t frames;

	frames = snd_pcm_writei(playback_handle, buffer, num_frames);
	if (frames < 0) {
		frames = snd_pcm_recover(playback_handle, frames, 0);
	}
	if (frames < 0) {
		slError("snd_pcm_writei failed\n");
	}
}