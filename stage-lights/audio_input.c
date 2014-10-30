#include "audio_input.h"

#include "audio_in/file_in.h"
#include "audio_in/alsa_in.h"


static const SLAudioInputSource INPUT_SOURCE = SLAIS_ALSA;

void slAudioInputInitialize(SLConfiguration const * config)
{
	UNUSED(config);

	switch(INPUT_SOURCE) {
	case SLAIS_FILE:
		slSndFileOpen("../test/clips/909Tom X1.wav");
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
		slSndFileClose();
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
		slSndFileReadLooping(audio,SL_AUDIO_FRAMES_PER_VIDEO_FRAME,2);
		break;
	case SLAIS_ALSA:
		slAlsaRead();
		break;
	}
	//UNUSED(audio);
	//slFatal("BLARMMMMM: %d\n", 34);
}


