#ifndef AUDIO_DEVICE_H_INCLUDED
#define AUDIO_DEVICE_H_INCLUDED


#include "rasterblocks.h"


typedef enum {
    // Open for read
    RBADM_INPUT,
    // Open for read, but also output audio as it is read
    RBADM_INPUT_TEE,
    // Open for output only
    RBADM_OUTPUT,
} RBAudioDeviceMode;


// Note: if you want simultaneous input and output, the devices must share a
// clock source in the hardware, otherwise in general you must do some really
// fancy correction for the drift between the sample clocks. In ALSA you
// request sync'd input and output by opening a single device in the "duplex"
// mode. That's why there is only one device name parameter: in general
// opening two separate devices independently doesn't work.
bool rbAudioDeviceInitialize(char const * deviceName, RBAudioDeviceMode mode);
void rbAudioDeviceShutdown(void);

// Call this function if the device was opened with modes _INPUT or _INPUT_TEE
void rbAudioDeviceBlockingRead(RBRawAudio * pAudio);

// Call this function if the device was opened with mode _OUTPUT
void rbAudioDeviceBlockingWrite(RBRawAudio * pAudio);


#endif // AUDIO_DEVICE_H_INCLUDED
