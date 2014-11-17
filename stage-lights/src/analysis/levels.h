#include <stdio.h>
#include <math.h>

void slAnalysisBasicLevels(SLRawAudio const * audio, SLAnalyzedAudio * analysis, int num_frames, int channels)
{
		float frameSum = 0;
		float frameRMS = 0;
		for (int frame = 0; frame < num_frames; frame++) {
			float channelSum = 0;
			for (int channel = 0; channel < channels; channel++) {
				channelSum +=  audio->audio[frame][channel];
			}
			
			frameSum += channelSum * channelSum;
		}
		frameRMS = frameSum/num_frames;
		frameRMS = sqrt(frameRMS);
		
		analysis->bassEnergy = frameRMS;
		//printf("rms: %f\n",frameRMS);
}
