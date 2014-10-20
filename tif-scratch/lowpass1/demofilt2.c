//gcc demofilt2.c -o demofilt2 -lsndfile -lm

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sndfile.h>
SNDFILE*sf_inp,*sf_out;
SF_INFO sf_info_inp;
SF_INFO sf_info_out;

#define BUFFERSIZE 512
#define CUTOFF_LOW 300.0
#define CUTOFF_HIGH 150.0
main(){
	int i, numCh;
	sf_count_t total,read_len;

	float din[BUFFERSIZE*2];
	float dout[BUFFERSIZE*2];
	float tmp[BUFFERSIZE*2];

	FILE *out;

	sf_inp=sf_open("clip00.wav",SFM_READ,&sf_info_inp);
	numCh = sf_info_inp.channels;
	sf_info_out.channels = numCh;
	printf("format: %X\n", sf_info_inp.format);
	printf("samprate: %d\n", sf_info_inp.samplerate);
	printf("frames: %d\n", (int)sf_info_inp.frames);
	sf_info_out.samplerate=sf_info_inp.samplerate;
	sf_info_out.format=sf_info_inp.format;

	sf_out=sf_open("output.wav",SFM_WRITE,&sf_info_out);
	out = fopen("outlevels.out", "w");

	float RC_low = 1.0/(2.0*M_PI*CUTOFF_LOW);
	float dt = 1.0/sf_info_inp.samplerate;
	float alpha_low = dt/(RC_low+dt);
	float RC_high = 1.0/(2.0*M_PI*CUTOFF_HIGH);
	float alpha_high = RC_high/(RC_high+dt);
	
	total=0;

	while(total<sf_info_inp.frames){
		read_len=sf_readf_float(sf_inp,din,BUFFERSIZE);
		//printf("read_len: %d, ", (int)read_len);
		/*dout[0] = din[0];
		dout[1] = din[1];
		//low pass
		/*for(i=2;i<read_len*2;i++){
			dout[i] = dout[i-2] + (alpha_low*(din[i] - dout[i-2]));
			dout[i+1] = dout[i-1] + (alpha_low*(din[i+1] - dout[i-1]));
		}*/
		float frameRMS = 0;
		float frameSum = 0;

		//low pass
		tmp[0] = din[0];
		tmp[1] = din[1];
		for(i = numCh; i < (read_len*numCh) - 1; i+=2){
			tmp[i] = tmp[i-2] + (alpha_low*(din[i] - tmp[i-2]));
			tmp[i+1] = tmp[i-1] + (alpha_low*(din[i+1] - tmp[i-1]));
		}
		//high pass
		dout[0] = tmp[0];
		dout[1] = tmp[1];
		for(i = numCh; i < (read_len*numCh) - 1; i+=2){
			dout[i] = alpha_high*(dout[i-2] + tmp[i] - tmp[i-2]);
			dout[i+1] = alpha_high*(dout[i-1] + tmp[i+1] - tmp[i-1]);
			//average two channels and square?
			frameSum += (dout[i]*dout[i]) + (dout[i+1]*dout[i+1]);
		}
		frameRMS = frameSum/read_len;
		frameRMS = sqrt(frameRMS);
		fprintf(out, "%12.10f\n", frameRMS);
		sf_writef_float(sf_out,dout,read_len);
		total+=read_len;
	}
	sf_close(sf_inp);
	sf_close(sf_out);
}
