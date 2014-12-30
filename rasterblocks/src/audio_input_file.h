#ifndef AUDIO_INPUT_FILE_H_INCLUDED
#define AUDIO_INPUT_FILE_H_INCLUDED

#include "rasterblocks.h"
#include <sndfile.h>

SNDFILE* rbSndFileOpen(char *file_name);
void rbSndFileClose();
/* Read from audio file and seek to beggining when file ends */
void rbSndFileReadLooping(RBRawAudio* audio_buf, int num_frames, int channels);

#endif