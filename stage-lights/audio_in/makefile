JACK_OUT = jackrec
ALSA_OUT = alsa_loop

#CROSS_COMPILE = arm-linux-gnueabi-

CC_C = $(CROSS_COMPILE)gcc

CFLAGS = -Wall -g -std=c99 -D_POSIX_C_SOURCE=200809L -Werror
ALSA_LIBS = -lasound
JACK_LIBS = -ljack -lpthread -lsndfile
all: alsa_test jack_test

alsa_test:
	$(CC_C) $(CFLAGS) loop.c -o $(ALSA_OUT) $(ALSA_LIBS)

jack_test:
	$(CC_C) $(CFLAGS) capture_client.c -o $(JACK_OUT) $(JACK_LIBS)

clean:
	rm -rf $(ALSA_OUT) $(JACK_OUT)