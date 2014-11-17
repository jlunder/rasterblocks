#ifndef AUDIO_INPUT_ALSA_H_INCLUDED
#define AUDIO_INPUT_ALSA_H_INCLUDED

#include "stage_lights.h"

int slAlsaCaptureInit(const char* playback,const char* capture,unsigned int num_frames,unsigned int channels,unsigned int rate);
void slAlsaCaptureClose();
void slAlsaRead(SLRawAudio* audio_buf);


void slAlsaPlaybackInit(int num_frames, int channels);
void slAlsaPlaybackClose();
void slAlsaPlayback(SLRawAudio* audio_buf, int num_frames, int channels);

#endif