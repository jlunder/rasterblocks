#include "audio_input.h"

#include "audio_input_file.h"
#include "audio_alsa.h"


static SLAudioInputSource g_slAudioSource = SLAIS_INVALID;
static uint64_t g_slAudioVideoFrameCount = 0;
static bool g_monitorAudio = false;


void slAudioInputInitialize(SLConfiguration const * config)
{
    // In case of reinit, shut down first
    slAudioInputShutdown();
    
    switch(config->audioSource) {
    case SLAIS_ALSA:
        slAlsaCaptureInit(config->audioSourceParam, config->audioSourceParam,
            SL_AUDIO_FRAMES_PER_VIDEO_FRAME,SL_AUDIO_CHANNELS,
            SL_AUDIO_SAMPLE_RATE);
        g_slAudioSource = SLAIS_ALSA;
        break;
    case SLAIS_FILE:
        slSndFileOpen((char *)config->audioSourceParam);
        g_slAudioSource = SLAIS_FILE;
        break;
    default:
    	slFatal("Invalid audio source type %d\n", config->audioSource);
    	break;
    }
    if(config->monitorAudio) {
        g_monitorAudio = true;
        slAlsaPlaybackInit(SL_AUDIO_FRAMES_PER_VIDEO_FRAME,SL_AUDIO_CHANNELS);
    }
}


void slAudioInputShutdown(void)
{
    switch(g_slAudioSource) {
    case SLAIS_INVALID:
    	// Do nothing! We have already been shut down properly, or maybe we
    	// were never init'd.
    	break;
    case SLAIS_ALSA:
        slAlsaCaptureClose();
        break;
    case SLAIS_FILE:
        slSndFileClose();
        break;
    default:
    	slFatal("Invalid audio source type %d\n", g_slAudioSource);
    	break;
    }
    if(g_monitorAudio) {
        slAlsaPlaybackClose();
    }
    
    g_slAudioSource = SLAIS_INVALID;
}


void slAudioInputBlockingRead(SLRawAudio * audio)
{
    switch(g_slAudioSource) {
    case SLAIS_ALSA:
        slAlsaRead(audio);
        break;
    case SLAIS_FILE:
        slSndFileReadLooping(audio, SL_AUDIO_FRAMES_PER_VIDEO_FRAME,
            SL_AUDIO_CHANNELS);
        break;
    default:
    	// SLAIS_INVALID is not legal here: we must be init'd
    	slFatal("Invalid audio source type %d\n", g_slAudioSource);
    	break;
    }

    if(g_monitorAudio) {
        slAlsaPlayback(audio, SL_AUDIO_FRAMES_PER_VIDEO_FRAME, 
            SL_AUDIO_CHANNELS);
    }
    
    if(slInfoEnabled()) {
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
        slInfo("Audio for video frame %llu; peak = %.4f, power = %.4f\n",
            g_slAudioVideoFrameCount, peak, totalPower);
    }
    
    ++g_slAudioVideoFrameCount;
}

