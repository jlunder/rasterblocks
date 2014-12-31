#ifdef RB_USE_OPENAL_DEVICE

#include "audio_device.h"

#include <OpenAL/al.h>
#include <OpenAL/alc.h>


static ALCdevice * g_rbPOpenALDevice = NULL;


bool rbAudioDeviceInitialize(char const * deviceName, RBAudioDeviceMode mode)
{
    UNUSED(mode);
    
    rbAudioDeviceShutdown();
    
    alGetError();
    if(deviceName != NULL && deviceName[0] == '\0') {
        deviceName = NULL;
    }
    g_rbPOpenALDevice = alcCaptureOpenDevice(deviceName, RB_AUDIO_SAMPLE_RATE,
        AL_FORMAT_STEREO16, RB_AUDIO_FRAMES_PER_VIDEO_FRAME * 2);
    if (alGetError() != AL_NO_ERROR) {
        rbAudioDeviceShutdown();
        return false;
    }
    
    alcCaptureStart(g_rbPOpenALDevice);

    return true;
}


void rbAudioDeviceShutdown(void)
{
    if(g_rbPOpenALDevice == NULL) {
        return;
    }
    
    alcCaptureStop(g_rbPOpenALDevice);
    alcCaptureCloseDevice(g_rbPOpenALDevice);
    g_rbPOpenALDevice = NULL;
}


void rbAudioDeviceBlockingRead(RBRawAudio * pAudio)
{
    ALshort buf[RB_AUDIO_FRAMES_PER_VIDEO_FRAME][2];
    
    for(size_t i = 0; ; ++i) {
        ALint samplesAvail = 0;
        alcGetIntegerv(g_rbPOpenALDevice, ALC_CAPTURE_SAMPLES,
            (ALCsizei)sizeof(ALint), &samplesAvail);
        if(samplesAvail >= RB_AUDIO_FRAMES_PER_VIDEO_FRAME) {
            break;
        }
        rbAssert(i < 100);
        rbInfo("Audio read: %d samples, need %d\n", samplesAvail,
            RB_AUDIO_FRAMES_PER_VIDEO_FRAME);
        rbSleep(rbTimeFromMs(1000 / (RB_VIDEO_FRAME_RATE * 2)));
    }
    
    alcCaptureSamples(g_rbPOpenALDevice, (ALCvoid *)buf,
        RB_AUDIO_FRAMES_PER_VIDEO_FRAME);
    for(size_t i = 0; i < RB_AUDIO_FRAMES_PER_VIDEO_FRAME; ++i) {
        for(size_t j = 0; j < RB_AUDIO_CHANNELS; ++j) {
            pAudio->audio[i][j] = (float)buf[i][j % 2] * (1.0f / 32768);
        }
    }
}


void rbAudioDeviceBlockingWrite(RBRawAudio * pAudio)
{
    UNUSED(pAudio);
}


#endif
