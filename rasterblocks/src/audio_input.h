#ifndef AUDIO_INPUT_H_INCLUDED
#define AUDIO_INPUT_H_INCLUDED


#include "rasterblocks.h"


// Audio input subsystem
void rbAudioInputInitialize(RBConfiguration const * config);
void rbAudioInputShutdown(void);
void rbAudioInputBlockingRead(RBRawAudio * audio);


#endif

