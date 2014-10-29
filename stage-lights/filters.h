#ifndef FILTERS_H_INCLUDED
#define FILTERS_H_INCLUDED
#include "stage_lights.h"

void filter_analyze(SLRawAudio const * audio, SLAnalyzedAudio * analysis, int num_frames, int channels);

#endif