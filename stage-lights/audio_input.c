#include "audio_input.h"

#include "audio_in/file_in.h"

SNDFILE* testwav;

void slAudioInputInitialize(SLConfiguration const * config)
{
	UNUSED(config);

	testwav = audio_file_open("../test/clips/909Tom X1.wav");
}


void slAudioInputShutdown(void)
{
	audio_file_close(testwav);
}


void slAudioInputBlockingRead(SLRawAudio * audio)
{
	//UNUSED(audio);
	audio_file_read_looping(audio,testwav,SL_AUDIO_FRAMES_PER_VIDEO_FRAME,2);
	slFatal("BLARMMMMM: %d\n", 34);
}


