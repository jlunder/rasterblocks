#ifndef AUDIO_INPUT_H_INCLUDED
#define AUDIO_INPUT_H_INCLUDED


#include "stage_lights.h"


// Audio input subsystem
void slAudioInputInitialize(SLConfiguration const * config);
void slAudioInputShutdown(void);
void slAudioInputBlockingRead(SLRawAudio * audio);


#endif

