#include "audio_input.h"


#include "audio_file.h"
#include "audio_device.h"


static RBAudioInputSource g_rbAudioSource = RBAIS_INVALID;
static uint64_t g_rbAudioVideoFrameCount = 0;


void rbAudioInputInitialize(RBConfiguration const * config)
{
    // In case of reinit, shut down first
    rbAudioInputShutdown();
    
    switch(config->audioSource) {
    case RBAIS_DEVICE:
        rbVerify(rbAudioDeviceInitialize(config->audioSourceParam,
            config->monitorAudio ? RBADM_INPUT_TEE : RBADM_INPUT));
        g_rbAudioSource = RBAIS_DEVICE;
        break;
    case RBAIS_FILE:
        rbVerify(rbAudioFileInitialize((char *)config->audioSourceParam));
        rbAudioDeviceInitialize(NULL, RBADM_OUTPUT);
        g_rbAudioSource = RBAIS_FILE;
        break;
    default:
        g_rbAudioSource = RBAIS_INVALID;
    	rbFatal("Invalid audio source type %d\n", config->audioSource);
    	break;
    }
}


void rbAudioInputShutdown(void)
{
    switch(g_rbAudioSource) {
    case RBAIS_INVALID:
    	// Do nothing! We have already been shut down properly, or maybe we
    	// were never init'd.
    	break;
    case RBAIS_DEVICE:
        rbAudioDeviceShutdown();
        break;
    case RBAIS_FILE:
        rbAudioDeviceShutdown();
        rbAudioFileShutdown();
        break;
    default:
    	rbFatal("Invalid audio source type %d\n", g_rbAudioSource);
    	break;
    }
    
    g_rbAudioSource = RBAIS_INVALID;
}


void rbAudioInputBlockingRead(RBRawAudio * audio)
{
    switch(g_rbAudioSource) {
    case RBAIS_DEVICE:
        rbAudioDeviceBlockingRead(audio);
        break;
    case RBAIS_FILE:
        rbAudioFileReadLooping(audio);
        break;
    default:
    	// RBAIS_INVALID is not legal here: we must be init'd
    	rbFatal("Invalid audio source type %d\n", g_rbAudioSource);
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
    rbFatal("Not compiled with libsndfile support!\n");
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
