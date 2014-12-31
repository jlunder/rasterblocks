#ifdef RB_USE_ALSA_DEVICE

#include "audio_device.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <alsa/asoundlib.h>
#include <signal.h>


static snd_pcm_t *g_rbAlsaCaptureHandle = NULL;


static bool rbAlsaCaptureInit(const char* captureDev);
static bool rbOpenStream(snd_pcm_t **handle, const char *name, int dir,
    unsigned int format, unsigned int channels, unsigned int rate,
    int bufferSize);
static void rbAlsaCaptureClose();
static void rbAlsaRead(RBRawAudio* audio);


bool rbAudioDeviceInitialize(char const * deviceName, RBAudioDeviceMode mode)
{
    UNUSED(mode);
    return rbAlsaCaptureInit(deviceName);
}


void rbAudioDeviceShutdown(void)
{
    rbAlsaCaptureClose();
}


void rbAudioDeviceBlockingRead(RBRawAudio * pAudio)
{
    rbAlsaRead(pAudio);
}


void rbAudioDeviceBlockingWrite(RBRawAudio * pAudio)
{
    UNUSED(pAudio);
}


bool rbAlsaCaptureInit(const char * captureDev)
{
    int err;
    
    if(!rbOpenStream(&g_rbAlsaCaptureHandle, captureDev,
            SND_PCM_STREAM_CAPTURE, SND_PCM_FORMAT_FLOAT_LE,
            RB_AUDIO_CHANNELS, RB_AUDIO_SAMPLE_RATE,
            RB_AUDIO_FRAMES_PER_VIDEO_FRAME)) {
        if(rbIsRestarting()) {
            // Special case: we managed to init once before, so let's believe
            // the failure might go away if we just try again. In this case we
            // want to leave the device uninitialized and request another
            // restart in the *Process().
            g_rbAlsaCaptureHandle = NULL;
            return true;
        }
        else {
            return false;
        }
    }

    err = snd_pcm_start(g_rbAlsaCaptureHandle);
    if (err < 0) {
        rbError("cannot prepare audio interface for use(%s)\n",
            snd_strerror(err));
        return false;
    }
    
    return true;
}


static bool rbOpenStream(snd_pcm_t **handle, const char *name, int dir,
    unsigned int format, unsigned int channels, unsigned int rate,
    int bufferSize)
{
    snd_pcm_hw_params_t *pHwParams = NULL;
    snd_pcm_sw_params_t *pSwParams = NULL;
    char const * failMessage = "unexpected failure";
    int err = 0;

    // Please excuse the idiomatic error-checking: the straightforward way was
    // too verbose to be readable.
    // The idea here is at each step we check whether an error was previously
    // encountered, so as soon as an error occurs, none of the following steps
    // are executed, and we fall directly through to the error-handling code
    // at the end of the function.
    
    // The first (large) stanza deals with the hardware parameters.
    if(err >= 0) {
        err = snd_pcm_open(handle, name, dir, 0);
        failMessage = "cannot open audio device";
    }
    if(err >= 0) {
        err = snd_pcm_hw_params_malloc(&pHwParams);
        failMessage = "cannot allocate hardware parameter structure";
        if(err < 0) {
            pHwParams = NULL;
        }
    }
    if(err >= 0) {
        err = snd_pcm_hw_params_any(*handle, pHwParams);
        failMessage = "cannot initialize hardware parameter structure";
    }
    if(err >= 0) {
        err = snd_pcm_hw_params_set_access(*handle, pHwParams,
            SND_PCM_ACCESS_RW_INTERLEAVED);
        failMessage = "cannot set access type";
    }
    if(err >= 0) {
        err = snd_pcm_hw_params_set_format(*handle, pHwParams, format);
        failMessage = "cannot set sample format";
    }
    if(err >= 0) {
        snd_pcm_hw_params_set_rate_near(*handle, pHwParams, &rate, NULL);
        failMessage = "cannot set sample rate";
    }
    if(err >= 0) {
        snd_pcm_hw_params_set_channels(*handle, pHwParams, channels);
        failMessage = "cannot set channel count";
    }
    if(err >= 0) {
        unsigned int bufferTime = 100000; // In us, so this is 100ms
        err = snd_pcm_hw_params_set_buffer_time_max(*handle, pHwParams,
            &bufferTime, 0);
        failMessage = "cannot set max buffer time";
    }
    if(err >= 0) {
        snd_pcm_hw_params(*handle, pHwParams);
        failMessage = "cannot set parameters";
    }

    // This second (large) stanza deals with software parameters.
    if(err >= 0) {
        err = snd_pcm_sw_params_malloc(&pSwParams);
        failMessage = "cannot allocate software parameters structure";
    }
    if(err >= 0) {
        err = snd_pcm_sw_params_current(*handle, pSwParams);
        failMessage = "cannot initialize software parameters structure";
    }
    if(err >= 0) {
        err = snd_pcm_sw_params_set_avail_min(*handle, pSwParams,
            bufferSize);
        failMessage = "cannot set minimum available count";
    }
    if(err >= 0) {
        err = snd_pcm_sw_params_set_start_threshold(*handle, pSwParams, 0U);
        failMessage = "cannot set start mode";
    }
    if(err >= 0) {
        err = snd_pcm_sw_params(*handle, pSwParams);
        failMessage = "cannot set software parameters";
    }
    
    // Free any resources allocated by the function before we leave.
    if(pHwParams != NULL) {
        snd_pcm_hw_params_free(pHwParams);
    }
    if(pSwParams != NULL) {
        snd_pcm_sw_params_free(pSwParams);
    }
    
    // This code handles all errors generated above, uniformly.
    if (err < 0) {
        rbError("%s (%s): %s (%s)\n", name,
            (dir == SND_PCM_STREAM_PLAYBACK) ? "PLAYBACK" : "CAPTURE",
            failMessage, snd_strerror(err));
        return false;
    }
    else {
        return true;
    }
}


void rbAlsaCaptureClose() {
    if(g_rbAlsaCaptureHandle != NULL) {
        rbInfo("Closing alsa capture device\n");
        snd_pcm_close(g_rbAlsaCaptureHandle);
        g_rbAlsaCaptureHandle = NULL;
    }
}


void rbAlsaRead(RBRawAudio * pAudio) {
    snd_pcm_sframes_t framesRead;
    
    if(g_rbAlsaCaptureHandle == NULL) {
        // Init failure -- probably audio device is not plugged in.
        // If we get here it's because the init function wants another delayed
        // restart, but it can't request one because a failed init causes
        // immediate shutdown.
        memset(pAudio->audio, 0, sizeof pAudio->audio);
        rbRequestDelayedGentleRestart();
        return;
    }
    
    snd_pcm_wait(g_rbAlsaCaptureHandle, 1000);
    
    switch(snd_pcm_state(g_rbAlsaCaptureHandle)) {
    case SND_PCM_STATE_OPEN:
    case SND_PCM_STATE_SETUP:
    case SND_PCM_STATE_XRUN:
    case SND_PCM_STATE_PAUSED:
    case SND_PCM_STATE_SUSPENDED:
        snd_pcm_prepare(g_rbAlsaCaptureHandle);
        break;
    default:
        break;
    }
    
    framesRead = snd_pcm_readi(g_rbAlsaCaptureHandle, pAudio->audio,
        RB_AUDIO_FRAMES_PER_VIDEO_FRAME);
    if(framesRead < 0) {
        rbError("readi failed (%s)\n", snd_strerror(framesRead));
        memset(pAudio->audio, 0, sizeof pAudio->audio);
        if(framesRead == -ENODEV) {
            // In this circumstance, we may be recovering from an unplugged
            // audio device -- we need a pause to give time for the user to
            // plug the device back in.
            rbRequestDelayedGentleRestart();
        }
        else {
            rbRequestGentleRestart();
        }
    }
}


#endif
