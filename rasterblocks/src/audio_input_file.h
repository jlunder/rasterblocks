#ifndef AUDIO_INPUT_FILE_H_INCLUDED
#define AUDIO_INPUT_FILE_H_INCLUDED

#include "rasterblocks.h"
#include <sndfile.h>

SNDFILE* slSndFileOpen(char *file_name);
void slSndFileClose();
/* Read from audio file and seek to beggining when file ends */
void slSndFileReadLooping(SLRawAudio* audio_buf, int num_frames, int channels);

#endif