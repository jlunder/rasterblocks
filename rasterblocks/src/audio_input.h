#ifndef AUDIO_INPUT_H_INCLUDED
#define AUDIO_INPUT_H_INCLUDED


#include "rasterblocks.h"


// Audio input subsystem
void rbAudioInputInitialize(RBConfiguration const * pConfig);
void rbAudioInputShutdown(void);
void rbAudioInputBlockingRead(RBRawAudio * pAudio);


void rbAudioInputPrussInitialize(RBConfiguration const * pConfig);
void rbAudioInputPrussShutdown(void);
void rbAudioInputPrussBlockingRead(RBRawAudio * pAudio);


#endif

