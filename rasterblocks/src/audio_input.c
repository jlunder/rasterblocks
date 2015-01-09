#include "audio_input.h"


#include "audio_file.h"
#include "audio_device.h"


static RBAudioInput g_rbAudioInput = RBAI_INVALID;
static uint64_t g_rbAudioVideoFrameCount = 0;


void rbAudioInputInitialize(RBConfiguration const * config)
{
    // In case of reinit, shut down first
    rbAudioInputShutdown();
    
    switch(config->audioInput) {
    case RBAI_ALSA:
    case RBAI_OPENAL:
        rbVerify(rbAudioDeviceInitialize(config->audioInputParam,
            RBADM_INPUT));
        g_rbAudioInput = RBAI_ALSA;
        break;
    case RBAI_FILE:
        rbVerify(rbAudioFileInitialize((char *)config->audioInputParam));
        rbAudioDeviceInitialize(NULL, RBADM_OUTPUT);
        g_rbAudioInput = RBAI_FILE;
        break;
    default:
        g_rbAudioInput = RBAI_INVALID;
    	rbFatal("Invalid audio input type %d\n", config->audioInput);
    	break;
    }
}


void rbAudioInputShutdown(void)
{
    switch(g_rbAudioInput) {
    case RBAI_INVALID:
    	// Do nothing! We have already been shut down properly, or maybe we
    	// were never init'd.
    	break;
    case RBAI_ALSA:
    case RBAI_OPENAL:
        rbAudioDeviceShutdown();
        break;
    case RBAI_FILE:
        rbAudioDeviceShutdown();
        rbAudioFileShutdown();
        break;
    default:
    	rbFatal("Invalid audio source type %d\n", g_rbAudioInput);
    	break;
    }
    
    g_rbAudioInput = RBAI_INVALID;
}


void rbAudioInputBlockingRead(RBRawAudio * audio)
{
    switch(g_rbAudioInput) {
    case RBAI_ALSA:
    case RBAI_OPENAL:
        rbAudioDeviceBlockingRead(audio);
        break;
    case RBAI_FILE:
        rbAudioFileReadLooping(audio);
        break;
    default:
    	// RBAIS_INVALID is not legal here: we must be init'd
    	rbFatal("Invalid audio source type %d\n", g_rbAudioInput);
    	break;
    }
    
    if(rbInfoEnabled()) {
        float totalPower = 0.0f;
        float peak = 0.0f;
        for(size_t i = 0; i < LENGTHOF(audio->audio); ++i) {
            for(size_t j = 0; j < LENGTHOF(audio->audio[i]); ++j) {
                float s = audio->audio[i][j];
                if(fabsf(s) > peak) {
                    peak = fabsf(s);
                }
                totalPower += s * s;
            }
        }
        totalPower = sqrtf(totalPower / LENGTHOF(audio->audio));
        rbInfo("Audio for video frame %llu; peak = %.4f, power = %.4f\n",
            g_rbAudioVideoFrameCount, peak, totalPower);
    }
    
    ++g_rbAudioVideoFrameCount;
}


#ifndef RB_USE_SNDFILE_INPUT
bool rbAudioFileInitialize(char const * filename)
{
    UNUSED(filename);
    rbFatal("libsndfile input not included in this build!\n");
    return false;
}


void rbAudioFileShutdown(void)
{
}


void rbAudioFileReadLooping(RBRawAudio * pAudio)
{
    UNUSED(pAudio);
}
#endif
