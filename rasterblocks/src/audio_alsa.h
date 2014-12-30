#ifndef AUDIO_INPUT_ALSA_H_INCLUDED
#define AUDIO_INPUT_ALSA_H_INCLUDED

#include "rasterblocks.h"

bool slAlsaCaptureInit(const char* captureDev);
void slAlsaCaptureClose();
/* Blocking read from capture device */
void slAlsaRead(SLRawAudio* audio);


void slAlsaPlaybackInit(int num_frames, int channels);
void slAlsaPlaybackClose();
/* Playback for debugging, uses SLRawAudio so it can be used after alsa or file read */
void slAlsaPlayback(SLRawAudio* audio_buf, int num_frames, int channels);

#endif
