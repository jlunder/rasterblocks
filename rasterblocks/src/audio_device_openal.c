#ifdef RB_USE_OPENAL_DEVICE

#include "audio_device.h"


bool rbAudioDeviceInitialize(char const * deviceName, RBAudioDeviceMode mode)
{
    UNUSED(deviceName);
    UNUSED(mode);
    return false;
}


void rbAudioDeviceShutdown(void)
{
}


void rbAudioDeviceBlockingRead(RBRawAudio * audio)
{
    UNUSED(audio);
}


void rbAudioDeviceBlockingWrite(RBRawAudio * audio)
{
    UNUSED(audio);
}


#endif
