#ifndef AUDIO_INPUT_SNDFILE_H_INCLUDED
#define AUDIO_INPUT_SNDFILE_H_INCLUDED


#include "rasterblocks.h"


bool rbAudioFileInitialize(char const * filename);
void rbAudioFileShutdown(void);

// Read from audio file and seek to beggining when file ends
void rbAudioFileReadLooping(RBRawAudio * pAudio);


#endif