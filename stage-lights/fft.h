#ifndef FFT_H_INCLUDED
#define FFT_H_INCLUDED

#include "stage_lights.h"

void fftInitialize();
void fftDestroy();

void fftProcess(SLRawAudio const * audio);

#endif