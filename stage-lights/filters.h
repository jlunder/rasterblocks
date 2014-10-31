#ifndef FILTERS_H_INCLUDED
#define FILTERS_H_INCLUDED
#include "stage_lights.h"

//returns levels for bass, mid, and treble
void slAnalsisFilterLevels(SLRawAudio const * audio, SLAnalyzedAudio * analysis, int num_frames, int channels);

#endif