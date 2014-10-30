#include "audio_input.h"

#include "audio_in/file_in.h"
#include "audio_in/loop.c"

SNDFILE* testwav;

static const SLAudioInputSource INPUT_SOURCE = SLAIS_ALSA;

void slAudioInputInitialize(SLConfiguration const * config)
{
	UNUSED(config);

	switch(INPUT_SOURCE) {
	case SLAIS_FILE:
		testwav = audio_file_open("../test/clips/909Tom X1.wav");
		break;
	case SLAIS_ALSA:
		slAlsaInit("hw:PCH","hw:PCH",SL_AUDIO_FRAMES_PER_VIDEO_FRAME,2);
		break;
	}
}


void slAudioInputShutdown(void)
{
	switch(INPUT_SOURCE) {
	case SLAIS_FILE:
		audio_file_close(testwav);
		break;
	case SLAIS_ALSA:
		slAlsaClose();
		break;
	}
}


void slAudioInputBlockingRead(SLRawAudio * audio)
{
	switch(INPUT_SOURCE) {
	case SLAIS_FILE:
		audio_file_read_looping(audio,testwav,SL_AUDIO_FRAMES_PER_VIDEO_FRAME,2);
		break;
	case SLAIS_ALSA:
		slAlsaRead();
		break;
	}
	//UNUSED(audio);
	//slFatal("BLARMMMMM: %d\n", 34);
}


