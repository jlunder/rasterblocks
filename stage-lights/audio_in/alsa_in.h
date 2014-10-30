#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <alsa/asoundlib.h>
#include <signal.h>

snd_pcm_t *playback_handle, *capture_handle;

static int slOpenStream(snd_pcm_t **handle, const char *name, int dir, unsigned int format,unsigned int channels,unsigned int rate,int buffer_size)
{
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	const char *dirname = (dir == SND_PCM_STREAM_PLAYBACK) ? "PLAYBACK" : "CAPTURE";
	int err;

	if ((err = snd_pcm_open(handle, name, dir, 0)) < 0) {
		slError("%s (%s): cannot open audio device (%s)\n", name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		slError("%s (%s): cannot allocate hardware parameter structure (%s)\n",
				name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_any(*handle, hw_params)) < 0) {
		slError("%s (%s): cannot initialize hardware parameter structure(%s)\n",
				name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_set_access(*handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		slError("%s (%s): cannot set access type(%s)\n",
				name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_set_format(*handle, hw_params, format)) < 0) {
		slError("%s (%s): cannot set sample format(%s)\n",
				name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_set_rate_near(*handle, hw_params, &rate, NULL)) < 0) {
		slError("%s (%s): cannot set sample rate(%s)\n",
				name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_set_channels(*handle, hw_params, channels)) < 0) {
		slError("%s (%s): cannot set channel count(%s)\n",
				name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params(*handle, hw_params)) < 0) {
		slError("%s (%s): cannot set parameters(%s)\n",
				name, dirname, snd_strerror(err));
		return err;
	}

	snd_pcm_hw_params_free(hw_params);

	if ((err = snd_pcm_sw_params_malloc(&sw_params)) < 0) {
		slError("%s (%s): cannot allocate software parameters structure(%s)\n",
				name, dirname, snd_strerror(err));
		return err;
	}
	if ((err = snd_pcm_sw_params_current(*handle, sw_params)) < 0) {
		slError("%s (%s): cannot initialize software parameters structure(%s)\n",
				name, dirname, snd_strerror(err));
		return err;
	}
	if ((err = snd_pcm_sw_params_set_avail_min(*handle, sw_params, buffer_size)) < 0) {
		slError("%s (%s): cannot set minimum available count(%s)\n",
				name, dirname, snd_strerror(err));
		return err;
	}
	if ((err = snd_pcm_sw_params_set_start_threshold(*handle, sw_params, 0U)) < 0) {
		slError("%s (%s): cannot set start mode(%s)\n",
				name, dirname, snd_strerror(err));
		return err;
	}
	if ((err = snd_pcm_sw_params(*handle, sw_params)) < 0) {
		slError("%s (%s): cannot set software parameters(%s)\n",
				name, dirname, snd_strerror(err));
		return err;
	}

	return 0;
}

int* buffer;
int g_num_frames;
int g_channels;

int slAlsaInit(char* playback,char* capture,int num_frames,int channels)
{
	g_num_frames = num_frames;
	g_channels = channels;
	int err;
	if ((err = slOpenStream(&playback_handle, playback, SND_PCM_STREAM_PLAYBACK,SND_PCM_FORMAT_S32_LE,channels,44100,num_frames)) < 0) {
		slFatal("failed to open stream for playback");
		return err;
	}

	if ((err = slOpenStream(&capture_handle, capture, SND_PCM_STREAM_CAPTURE,SND_PCM_FORMAT_S32_LE,channels,44100,num_frames)) < 0) {
		slFatal("failed to open stream for capture");
		return err;
	}

	if ((err = snd_pcm_prepare(playback_handle)) < 0) {
		slFatal("cannot prepare audio interface for use(%s)\n", snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_start(capture_handle)) < 0) {
		slFatal("cannot prepare audio interface for use(%s)\n", snd_strerror(err));
		return err;
	}

	buffer = malloc(num_frames*channels*sizeof(int));
	memset(buffer, 0, num_frames*channels*sizeof(int));
	return 0;
}

void slAlsaClose() {
	slInfo("Closing alsa playback/capture devices\n");
	snd_pcm_close(playback_handle);
	snd_pcm_close(capture_handle);

	free(buffer);
}
void slAlsaPlayback(int num_frames) {
	int avail = snd_pcm_avail_update(playback_handle);
	if (avail > 0) {
		if (avail > num_frames)
			avail = num_frames;

		snd_pcm_writei(playback_handle, buffer, avail);
	}
}
/*float convert(const int src)
{
    int i = src[2] << 24 | src[1] << 16 | src[0] << 8 ;
    return i / (float)(INT_MAX - 256) ;
}*/
//void slAlsaRead(SLRawAudio* audio_buf, int num_frames, int channels) {
void slAlsaRead() {

	int err;
	if ((err = snd_pcm_wait(playback_handle, 1000)) < 0) {
		slError("poll failed(%s)\n", strerror(errno));
	}

	int avail = snd_pcm_avail_update(capture_handle);
	if (avail > 0) {
		if (avail > g_num_frames)
			avail = g_num_frames;

		snd_pcm_readi(capture_handle, buffer, avail);
	}
	printf("available: %d nframes: %d\n",avail,g_num_frames);

	slAlsaPlayback(g_num_frames);
	/*for (int frame = 0; frame < num_frames; frame++) {
		for (int channel = 0; channel < channels; channel++) {
			audio_buf->audio[frame][channel] = (float)buffer[frame*channels+channel];
		}
	}
	printf("frame 1: %d %f\n",buffer[0],audio_buf->audio[0][0]);*/
}
