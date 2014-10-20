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
#define CUTOFF_HIGH 100.0

//#define SIMP
#define OTH

#define NZEROS 2
#define NPOLES 2
#define GAIN 2.255945996e+03

#ifdef OTH
	double cx[NZEROS+1];
	double cy[NPOLES+1];
	static double filter(double xv[NZEROS+1],double yv[NPOLES+1],double next)
	{
		xv[0]=xv[1];
		xv[1]=xv[2];
		xv[2]=next/GAIN;
		yv[0]=yv[1];
		yv[1]=yv[2];
		yv[2]=(xv[2] + xv[0]) + 2*xv[1] + (-0.9413432994 * yv[0]) + (1.9395702074 * yv[1]); 
		return yv[2];
	}
#endif


main(){
	int numCh;
	sf_count_t total,read_len;

	float din[BUFFERSIZE*2];
	float dout[BUFFERSIZE*2];
	float tmp[BUFFERSIZE*2];

	FILE *out, *out_tmp;

	sf_inp=sf_open("clip00.wav",SFM_READ,&sf_info_inp);
	numCh = sf_info_inp.channels;
	sf_info_out.channels = numCh;
	printf("format: %X\n", sf_info_inp.format);
	printf("samprate: %d\n", sf_info_inp.samplerate);
	printf("frames: %d\n", (int)sf_info_inp.frames);
	sf_info_out.samplerate=sf_info_inp.samplerate;
	sf_info_out.format=sf_info_inp.format;

	sf_out=sf_open("output.wav",SFM_WRITE,&sf_info_out);
	out_tmp = fopen("levels.out", "w");
	out = fopen("outlevels.out", "w");

	float RC_low = 1.0/(2.0*M_PI*CUTOFF_LOW);
	float dt = 1.0/sf_info_inp.samplerate;
	float alpha_low = dt/(RC_low+dt);
	float RC_high = 1.0/(2.0*M_PI*CUTOFF_HIGH);
	float alpha_high = RC_high/(RC_high+dt);
	
	total=0;

	while(total < sf_info_inp.frames){

		float frameRMS = 0;
		float frameSum = 0;
		float channelSum;
		int ch, f;

		//tmp
		float frameRMS_t = 0;
		float frameSum_t = 0;
		float channelSum_t;

		read_len = sf_readf_float(sf_inp, din, BUFFERSIZE);
		//printf("read_len: %d, ", (int)read_len);
		
		#ifdef SIMP
			//low pass
			tmp[0] = din[0];
			tmp[1] = din[1];
			for(f = 1; f < read_len; f++){
				for(ch = 0; ch < numCh; ch++){
					tmp[f*numCh+ch] = tmp[(f*numCh+ch) - numCh] + (alpha_low*(din[f*numCh+ch] - tmp[(f*numCh+ch) - numCh]));
					channelSum_t += din[f*numCh+ch];
				}
				frameSum_t += channelSum_t * channelSum_t;
			}
			//high pass
			dout[0] = tmp[0];
			dout[1] = tmp[1];
			channelSum = dout[0];
			for(f = 1; f < read_len; f++){
				for(ch = 0; ch < numCh; ch++){
					dout[f*numCh+ch] = alpha_high*(dout[(f*numCh+ch) - numCh] + tmp[f*numCh+ch] - tmp[(f*numCh+ch)-numCh]);
					channelSum += dout[f*numCh+ch];
				}
				//RMS
				frameSum += channelSum * channelSum;
			}


			sf_writef_float(sf_out,dout,read_len);

			//tmp
			frameRMS_t = frameSum_t/read_len;
			frameRMS_t = sqrt(frameRMS_t);
			fprintf(out_tmp, "%12.10f", frameRMS_t);
			fprintf (out_tmp, "\n") ;
			//
		#endif

		#ifdef OTH
			for(f = 0; f < read_len; f++){
				for(ch = 0; ch < numCh; ch++){
					dout[f*numCh+ch] = 2.0*filter(cx, cy, din[f*numCh+ch]);
					channelSum += dout[f*numCh+ch];
				}
				frameSum += channelSum * channelSum;
			}
			sf_writef_float(sf_out,dout,read_len);
		#endif

		frameRMS = frameSum/read_len;
		frameRMS = sqrt(frameRMS);
		fprintf(out, "%12.10f", frameRMS);
		fprintf (out, "\n") ;

		total+=read_len;
	}
	sf_close(sf_inp);
	sf_close(sf_out);
	fclose(out);
	fclose(out_tmp);
}
