#include "audio_input.h"


#include "audio_file.h"
#include "audio_device.h"


static RBAudioInput g_rbAudioInput = RBAI_INVALID;
static uint64_t g_rbAudioVideoFrameCount = 0;

static RBTimer g_rbAudioTestFrameTimer;
static RBTimer g_rbAudioTestHighBeatTimer;
static RBTimer g_rbAudioTestLowBeatTimer;
static size_t g_rbAudioTestHighSamplesSinceTrigger;
static size_t g_rbAudioTestLowSamplesSinceTrigger;
static float g_rbAudioTestHighPhase;
static float g_rbAudioTestLowPhase;


void rbAudioInputInitialize(RBConfiguration const * pConfig)
{
    // In case of reinit, shut down first
    rbAudioInputShutdown();
    
    switch(pConfig->audioInput) {
    case RBAI_TEST:
        g_rbAudioInput = RBAI_TEST;
        rbStartTimer(&g_rbAudioTestFrameTimer,
            rbTimeFromMs(1000 / RB_VIDEO_FRAME_RATE));
        rbStartTimer(&g_rbAudioTestHighBeatTimer, rbTimeFromMs(500 / 4));
        rbStartTimer(&g_rbAudioTestLowBeatTimer, rbTimeFromMs(500));
        g_rbAudioTestHighSamplesSinceTrigger = 0;
        g_rbAudioTestLowSamplesSinceTrigger = 0;
        g_rbAudioTestHighPhase = 0.0f;
        g_rbAudioTestLowPhase = 0.0f;
        break;
    case RBAI_ALSA:
    case RBAI_OPENAL:
        rbVerify(rbAudioDeviceInitialize(pConfig->audioInputParam,
            RBADM_INPUT));
        g_rbAudioInput = RBAI_ALSA;
        break;
    case RBAI_FILE:
        rbVerify(rbAudioFileInitialize((char *)pConfig->audioInputParam));
        rbAudioDeviceInitialize(NULL, RBADM_OUTPUT);
        g_rbAudioInput = RBAI_FILE;
        break;
    case RBAI_PRUSS:
        rbAudioInputPrussInitialize(pConfig);
        g_rbAudioInput = RBAI_PRUSS;
        break;
    default:
        g_rbAudioInput = RBAI_INVALID;
    	rbFatal("Invalid audio input type %d\n", pConfig->audioInput);
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
    case RBAI_TEST:
        break;
    case RBAI_ALSA:
    case RBAI_OPENAL:
        rbAudioDeviceShutdown();
        break;
    case RBAI_FILE:
        rbAudioDeviceShutdown();
        rbAudioFileShutdown();
        break;
    case RBAI_PRUSS:
        rbAudioInputPrussShutdown();
        break;
    default:
    	rbFatal("Invalid audio source type %d\n", g_rbAudioInput);
    	break;
    }
    
    g_rbAudioInput = RBAI_INVALID;
}


void rbAudioInputBlockingRead(RBRawAudio * pAudio)
{
    switch(g_rbAudioInput) {
    case RBAI_TEST:
        if(rbGetTimerPeriodsAndReset(&g_rbAudioTestHighBeatTimer) > 0) {
            g_rbAudioTestHighSamplesSinceTrigger = 0;
        }
        if(rbGetTimerPeriodsAndReset(&g_rbAudioTestLowBeatTimer) > 0) {
            g_rbAudioTestLowSamplesSinceTrigger = 0;
        }
        for(size_t i = 0; i < LENGTHOF(pAudio->audio); ++i) {
            g_rbAudioTestHighPhase = fmodf(g_rbAudioTestHighPhase +
                2.0f * RB_PI * 1000.0f / RB_AUDIO_SAMPLE_RATE, 2.0f * RB_PI);
            g_rbAudioTestLowPhase = fmodf(g_rbAudioTestLowPhase +
                2.0f * RB_PI * 50.0f / RB_AUDIO_SAMPLE_RATE, 2.0f * RB_PI);
            pAudio->audio[i][0] = pAudio->audio[i][1] =
                sinf(g_rbAudioTestHighPhase) * 0.3f *
                    powf(0.001f,
                        (float)g_rbAudioTestHighSamplesSinceTrigger /
                            RB_AUDIO_SAMPLE_RATE) +
                sinf(g_rbAudioTestHighPhase) * 0.5f *
                    powf(0.001f,
                        (float)g_rbAudioTestLowSamplesSinceTrigger /
                            RB_AUDIO_SAMPLE_RATE);
            ++g_rbAudioTestHighSamplesSinceTrigger;
            ++g_rbAudioTestLowSamplesSinceTrigger;
        }
        {
            RBTime timeLeft = rbGetTimeLeft(&g_rbAudioTestFrameTimer);
            if(timeLeft > 0) {
                rbSleep(timeLeft);
            }
            rbStartTimer(&g_rbAudioTestFrameTimer,
                rbTimeFromMs(1000 / RB_VIDEO_FRAME_RATE) + timeLeft);
        }
        break;
    case RBAI_ALSA:
    case RBAI_OPENAL:
        rbAudioDeviceBlockingRead(pAudio);
        break;
    case RBAI_FILE:
        rbAudioFileReadLooping(pAudio);
        break;
    case RBAI_PRUSS:
        rbAudioInputPrussBlockingRead(pAudio);
        break;
    default:
    	// RBAIS_INVALID is not legal here: we must be init'd
    	rbFatal("Invalid audio source type %d\n", g_rbAudioInput);
    	break;
    }
    
    if(rbInfoEnabled()) {
        float totalPower = 0.0f;
        float peak = 0.0f;
        for(size_t i = 0; i < LENGTHOF(pAudio->audio); ++i) {
            for(size_t j = 0; j < LENGTHOF(pAudio->audio[i]); ++j) {
                float s = pAudio->audio[i][j];
                if(fabsf(s) > peak) {
                    peak = fabsf(s);
                }
                totalPower += s * s;
            }
        }
        totalPower = sqrtf(totalPower / LENGTHOF(pAudio->audio));
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


#ifndef RB_USE_PRUSS_IO
void rbAudioInputPrussInitialize(RBConfiguration const * pConfig)
{
    UNUSED(pConfig);
    rbFatal("PRUSS audio input not included in this build!\n");
}


void rbAudioInputPrussShutdown(void)
{
}


void rbAudioInputPrussBlockingRead(RBRawAudio * pAudio)
{
    UNUSED(pAudio);
}
#endif // RB_USE_PRUSS_IO


