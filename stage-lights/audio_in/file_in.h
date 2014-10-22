
#include <sndfile.h>
#include <stdio.h>


SNDFILE* audio_file_open(char *file_name) {
	SNDFILE *inFile = NULL;
	SF_INFO  sfinfo;

	if ((inFile = sf_open(file_name, SFM_READ, &sfinfo)) == NULL) {
		printf("Not able to open input file %s.\n", file_name);
		return NULL;
	}
	return inFile;
}
void audio_file_close(SNDFILE* file) {
	sf_close(file);
}


static void audio_file_read_looping(SLRawAudio* audio_buf, SNDFILE* inFile, int num_frames, int channels)
{
	float buffer[channels * num_frames];
	int readCount = sf_readf_float(inFile, buffer, num_frames);
	for (int frame = 0; frame < num_frames; frame++) {
		for (int channel = 0; channel < channels; channel++) {
			audio_buf->audio[frame][channel] = buffer[frame*channels+channel];
		}
	}
	if(readCount==0)
		sf_seek(inFile,0,SEEK_SET);

	//printf("read: %i\n",readCount);
}
