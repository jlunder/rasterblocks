#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "filters.h"

#define NZEROS 2
#define NPOLES 2

//bass cutoff at 320
#define GAIN_B 2.347573845e+03
#define C0_B -0.9424820220
#define C1_B 1.9407781353
//mid in between
#define GAIN_M 3.751624790e+00
#define C0_M -0.5095254495
#define C1_M 1.4876623968
//treble cutoff at 5120
#define GAIN_T 1.613750167e+00
#define C0_T -0.3896437283
#define C1_T 1.0890546979

//store coefficients
static float cx_b[NZEROS+1], cy_b[NPOLES+1];
static float cx_m[NZEROS+1], cy_m[NPOLES+1];
static float cx_t[NZEROS+1], cy_t[NPOLES+1];

//bass
static float filterBass(float xv[NZEROS+1],float yv[NPOLES+1],float next)
{
	xv[0]=xv[1];
	xv[1]=xv[2];
	xv[2]=next/GAIN_B;
	yv[0]=yv[1];
	yv[1]=yv[2];
	yv[2]=(xv[0] + xv[2]) + 2*xv[1] + (C0_B * yv[0]) + (C1_B * yv[1]); 
	return yv[2];
}

//mid
static float filterMid(float xv[NZEROS+1],float yv[NPOLES+1],float next)
{
	xv[0]=xv[1];
	xv[1]=xv[2];
	xv[2]=next/GAIN_M;
	yv[0]=yv[1];
	yv[1]=yv[2];
	yv[2]=(xv[2] - xv[0]) + (C0_M * yv[0]) + (C1_M * yv[1]); 
	return yv[2];
}
//treble
static float filterTreble(float xv[NZEROS+1],float yv[NPOLES+1],float next)
{
	xv[0]=xv[1];
	xv[1]=xv[2];
	xv[2]=next/GAIN_T;
	yv[0]=yv[1];
	yv[1]=yv[2];
	yv[2]=(xv[0] + xv[2]) - 2*xv[1] + (C0_T * yv[0]) + (C1_T * yv[1]); 
	return yv[2];
}

static float calcRMS(float frameSum, int num_frames)
{
	float frameRMS = 0;
	frameRMS = frameSum/num_frames;
	frameRMS = sqrt(frameRMS);
	return frameRMS;
}
//filter function
void slAnalsisFilterLevels(SLRawAudio const * audio, SLAnalyzedAudio * analysis, int num_frames, int channels)
{
	float frameSum_b = 0;
	float frameSum_m = 0;
	float frameSum_t = 0;

	float frameRMS = 0;

	for(int frame = 0; frame < num_frames; frame++){
		float channelSum = 0;
		float filtered = 0;
		for(int ch = 0; ch < channels; ch++){
			channelSum += audio->audio[frame][ch];
		}
		channelSum /= channels;
		filtered = filterBass(cx_b, cy_b, channelSum);
		frameSum_b += filtered * filtered;
		filtered = filterMid(cx_m, cy_m, channelSum);
		frameSum_m += filtered * filtered;
		filtered = filterTreble(cx_t, cy_t, channelSum);
		frameSum_t += filtered * filtered;
	}

	frameRMS = calcRMS(frameSum_b, num_frames);
	//analysis->bassEnergy = frameRMS;

	frameRMS = calcRMS(frameSum_m, num_frames);
	analysis->midEnergy = frameRMS;

	frameRMS = calcRMS(frameSum_t, num_frames);
	analysis->trebleEnergy = frameRMS;
}
