#ifndef AUDIO_INPUT_ALSA_H_INCLUDED
#define AUDIO_INPUT_ALSA_H_INCLUDED

#include "rasterblocks.h"

bool rbAlsaCaptureInit(const char* captureDev);
void rbAlsaCaptureClose();
/* Blocking read from capture device */
void rbAlsaRead(RBRawAudio* audio);


void rbAlsaPlaybackInit(int num_frames, int channels);
void rbAlsaPlaybackClose();
/* Playback for debugging, uses RBRawAudio so it can be used after alsa or file read */
void rbAlsaPlayback(RBRawAudio* audio_buf, int num_frames, int channels);

#endif
