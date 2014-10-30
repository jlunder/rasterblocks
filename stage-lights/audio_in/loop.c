
#include <signal.h>

#define slError(...) printf(__VA_ARGS__);
#define slFatal(...) printf(__VA_ARGS__);
#define slInfo(...) printf(__VA_ARGS__);
#include "alsa_in.h"


void intHandler(int signal) {
	slAlsaClose();
	exit(0);
}
int main(int argc, char *argv[])
{
	signal(SIGINT, intHandler);

	slAlsaInit(argv[1],argv[2],480,2);
	while (1) {
		slAlsaRead();
	}
	slAlsaClose();

	return 0;
}

