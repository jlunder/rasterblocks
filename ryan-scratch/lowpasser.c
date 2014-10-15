#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <sndfile.h>

#define	BLOCK_SIZE 512

static void  monitor_levels(SNDFILE* inFile, FILE * outFile, int channels);

int main (int argc, char* argv[])
{
	char *inFileName = "fileIn.wav";
	char* outFileName = "fileOut.txt";

	SNDFILE *inFile = NULL;
	FILE *outFile = NULL;
	SF_INFO  sfinfo;

	if ((inFile = sf_open(inFileName, SFM_READ, &sfinfo)) == NULL) {
		printf("Not able to open input file %s.\n", inFileName);
		return 1;
	}

	if ((outFile = fopen(outFileName, "w")) == NULL) {
		printf("Not able to open output file %s : %s\n", outFileName, sf_strerror (NULL));
		return 1;
	}

	monitor_levels(inFile, outFile, sfinfo.channels);

	sf_close(inFile);

	return 0;
}

static void  monitor_levels(SNDFILE* inFile, FILE * outFile, int channels)
{
	float bufffer[channels * BLOCK_SIZE];
	int readCount;

	while ((readCount = sf_readf_float(inFile, bufffer, BLOCK_SIZE)) > 0) {
		float frameSum = 0;
		float frameRMS = 0;
		// Filtering is done here; maybe generate an intermediate buffer
		////FILTER
		// Performs RMS averaging. 
		for (int i = 0; i < readCount; i++) {
			float channelSum = 0;
			for (int j = 0; j < channels; j++) {
				channelSum +=  bufffer[i*channels+j];
			}
			
			frameSum += channelSum * channelSum;
		}
		frameRMS = frameSum/readCount;
		frameRMS = sqrt(frameRMS);
		fprintf(outFile, " %12.10f", frameRMS);
		fprintf (outFile, "\n") ;
	}

	return;
}
