#include "audio_input.h"


#ifdef RB_SNDFILE_SUPPORTED
#include "audio_input_file.h"
#endif

#include "audio_alsa.h"


static RBAudioInputSource g_slAudioSource = RBAIS_INVALID;
static uint64_t g_slAudioVideoFrameCount = 0;
static bool g_monitorAudio = false;


void rbAudioInputInitialize(RBConfiguration const * config)
{
    // In case of reinit, shut down first
    rbAudioInputShutdown();
    
    switch(config->audioSource) {
    case RBAIS_ALSA:
        rbAlsaCaptureInit(config->audioSourceParam);
        g_slAudioSource = RBAIS_ALSA;
        break;
    case RBAIS_FILE:
        #ifdef RB_SNDFILE_SUPPORTED
            rbVerify(rbSndFileOpen((char *)config->audioSourceParam) != NULL);
            g_slAudioSource = RBAIS_FILE;
        #else
            g_slAudioSource = RBAIS_INVALID;
            rbFatal("Not compiled with libsndfile support\n");
        #endif
        break;
    default:
        g_slAudioSource = RBAIS_INVALID;
    	rbFatal("Invalid audio source type %d\n", config->audioSource);
    	break;
    }
    
    if(config->monitorAudio) {
        g_monitorAudio = true;
        rbAlsaPlaybackInit(RB_AUDIO_FRAMES_PER_VIDEO_FRAME,RB_AUDIO_CHANNELS);
    }
}


void rbAudioInputShutdown(void)
{
    switch(g_slAudioSource) {
    case RBAIS_INVALID:
    	// Do nothing! We have already been shut down properly, or maybe we
    	// were never init'd.
    	break;
    case RBAIS_ALSA:
        rbAlsaCaptureClose();
        break;
    case RBAIS_FILE:
        #ifdef RB_SNDFILE_SUPPORTED
            rbSndFileClose();
        #endif
        break;
    default:
    	rbFatal("Invalid audio source type %d\n", g_slAudioSource);
    	break;
    }
    if(g_monitorAudio) {
        rbAlsaPlaybackClose();
    }
    
    g_slAudioSource = RBAIS_INVALID;
}


void rbAudioInputBlockingRead(RBRawAudio * audio)
{
    switch(g_slAudioSource) {
    case RBAIS_ALSA:
        rbAlsaRead(audio);
        break;
    case RBAIS_FILE:
        #ifdef RB_SNDFILE_SUPPORTED
            rbSndFileReadLooping(audio, RB_AUDIO_FRAMES_PER_VIDEO_FRAME,
                RB_AUDIO_CHANNELS);
        #else
            rbFatal("Not compiled with libsndfile support, exiting\n");
        #endif
        break;
    default:
    	// RBAIS_INVALID is not legal here: we must be init'd
    	rbFatal("Invalid audio source type %d\n", g_slAudioSource);
    	break;
    }

    if(g_monitorAudio) {
        rbAlsaPlayback(audio, RB_AUDIO_FRAMES_PER_VIDEO_FRAME, 
            RB_AUDIO_CHANNELS);
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
            g_slAudioVideoFrameCount, peak, totalPower);
    }
    
    ++g_slAudioVideoFrameCount;
}


