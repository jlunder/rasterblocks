#ifdef SL_SNDFILE_SUPPORTED

#include "audio_input_file.h"
#include <sndfile.h>
#include <stdio.h>

SNDFILE* g_snd_file;

SNDFILE* slSndFileOpen(char *file_name) {
	SF_INFO  sfinfo;

	if ((g_snd_file = sf_open(file_name, SFM_READ, &sfinfo)) == NULL) {
		printf("Not able to open input file %s.\n", file_name);
		return NULL;
	}

	return g_snd_file;

}

void slSndFileClose() {
	sf_close(g_snd_file);
}


void slSndFileReadLooping(SLRawAudio* audio_buf, int num_frames, int channels)
{
	float buffer[channels * num_frames];
	int readCount;

    memset(audio_buf->audio, 0, sizeof audio_buf->audio);	
	readCount = sf_readf_float(g_snd_file, buffer, num_frames);
	if(readCount < num_frames && readCount >= 0) {
		sf_seek(g_snd_file,0,SEEK_SET);
		sf_readf_float(g_snd_file, buffer, num_frames - readCount);
	}
	for (int frame = 0; frame < num_frames; frame++) {
		for (int channel = 0; channel < channels; channel++) {
			audio_buf->audio[frame][channel] = buffer[frame*channels+channel];
		}
	}

	//printf("read: %i\n",readCount);
}

#endif