
#include <signal.h>
#include <sys/time.h>

// so that alsa_in compiles
#define slError(...) printf(__VA_ARGS__)
#define slFatal(...) printf(__VA_ARGS__)
#define slInfo(...) printf(__VA_ARGS__)
typedef struct {
    float audio[1][1];
} SLRawAudio;
#include "alsa_in.h"



struct timeval t1, t2;
void startTimer() {
	gettimeofday(&t1, NULL);
}
void endTimer() {
	// stop timer
	gettimeofday(&t2, NULL);

	// compute and print the elapsed time in millisec
	double elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
	printf("elapsed time:%f ms\n",elapsedTime);
}

void intHandler(int signal) {
	slAlsaClose();
	exit(0);
}
int main(int argc, char *argv[])
{
	signal(SIGINT, intHandler);

	if(slAlsaInit(argv[1],argv[2],480,2,44100)) {
		return 1;
	}
	while (1) {
		startTimer();
		slAlsaRead();
		slAlsaPlayback();
		endTimer();
	}
	slAlsaClose();

	return 0;
}
