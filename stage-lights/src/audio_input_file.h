#ifndef AUDIO_INPUT_FILE_H_INCLUDED
#define AUDIO_INPUT_FILE_H_INCLUDED

#include "stage_lights.h"
#include <sndfile.h>

SNDFILE* slSndFileOpen(char *file_name);
void slSndFileClose();
void slSndFileReadLooping(SLRawAudio* audio_buf, int num_frames, int channels);

#endif