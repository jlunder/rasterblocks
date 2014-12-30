#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <alsa/asoundlib.h>
#include <signal.h>

#include "audio_alsa.h"

static snd_pcm_t *g_slAlsaCaptureHandle = NULL;

int mode = SND_PCM_NONBLOCK;
//int mode = SND_PCM_ASYNC;

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
        err = snd_pcm_open(handle, name, dir, mode);
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


bool rbAlsaCaptureInit(const char* captureDev)
{
    int err;
    
    if(!rbOpenStream(&g_slAlsaCaptureHandle, captureDev,
            SND_PCM_STREAM_CAPTURE, SND_PCM_FORMAT_FLOAT_LE,
            RB_AUDIO_CHANNELS, RB_AUDIO_SAMPLE_RATE,
            RB_AUDIO_FRAMES_PER_VIDEO_FRAME)) {
        if(rbIsRestarting()) {
            // Special case: we managed to init once before, so let's believe
            // the failure might go away if we just try again. In this case we
            // want to leave the device uninitialized and request another
            // restart in the *Process().
            g_slAlsaCaptureHandle = NULL;
            return true;
        }
        else {
            return false;
        }
    }

    err = snd_pcm_start(g_slAlsaCaptureHandle);
    if (err < 0) {
        rbError("cannot prepare audio interface for use(%s)\n",
            snd_strerror(err));
        return false;
    }
    
    return 0;
}

void rbAlsaCaptureClose() {
    if(g_slAlsaCaptureHandle != NULL) {
        rbInfo("Closing alsa capture device\n");
        snd_pcm_close(g_slAlsaCaptureHandle);
        g_slAlsaCaptureHandle = NULL;
    }
}

void rbAlsaRead(RBRawAudio* audio) {
    snd_pcm_sframes_t framesRead;
    
    if(g_slAlsaCaptureHandle == NULL) {
        // Init failure -- probably audio device is not plugged in.
        // If we get here it's because the init function wants another delayed
        // restart, but it can't request one because a failed init causes
        // immediate shutdown.
        memset(audio->audio, 0, sizeof audio->audio);
        rbRequestDelayedGentleRestart();
        return;
    }
    
    snd_pcm_wait(g_slAlsaCaptureHandle, 1000);
    
    switch(snd_pcm_state(g_slAlsaCaptureHandle)) {
    case SND_PCM_STATE_OPEN:
    case SND_PCM_STATE_SETUP:
    case SND_PCM_STATE_XRUN:
    case SND_PCM_STATE_PAUSED:
    case SND_PCM_STATE_SUSPENDED:
        snd_pcm_prepare(g_slAlsaCaptureHandle);
        break;
    default:
        break;
    }
    
    framesRead = snd_pcm_readi(g_slAlsaCaptureHandle, audio->audio,
        RB_AUDIO_FRAMES_PER_VIDEO_FRAME);
    if(framesRead < 0) {
        rbError("readi failed (%s)\n", snd_strerror(framesRead));
        memset(audio->audio, 0, sizeof audio->audio);
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


// ALSA playback

static snd_pcm_t *playback_handle;
float* playback_buffer;

void rbAlsaPlaybackInit(int num_frames, int channels)
{
    int error = 0;
    
    // Ensure all structures freed to prevent memory leaks
    rbAlsaPlaybackClose();
    
    error = snd_pcm_open(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK,
        0);

    if (error < 0 || playback_handle == NULL) {
        rbAlsaPlaybackClose();
        rbError("Alsa playback open failed\n");
        return;
    }

    error =  snd_pcm_set_params(playback_handle, SND_PCM_FORMAT_FLOAT,
                           SND_PCM_ACCESS_RW_INTERLEAVED, channels,
                           RB_AUDIO_SAMPLE_RATE, 1, 500000);
    if (error < 0) {
        rbAlsaPlaybackClose();
        rbError("Alsa playback set params failed\n");
        return;
    }

    playback_buffer = malloc(num_frames*channels*sizeof(*playback_buffer));
    // Alloc should never fail on a system with this much memory; if it does,
    // crashing is appropriate
    memset(playback_buffer, 0, num_frames*channels*sizeof(*playback_buffer));
}

void rbAlsaPlaybackClose()
{
    rbInfo("Closing alsa playback device\n");
    // Free and reset pointers
    if(playback_handle != NULL) {
        snd_pcm_close(playback_handle);
        playback_handle = NULL;
    }
    if(playback_buffer != NULL) {
        free(playback_buffer);
        playback_buffer = NULL;
    }
}

void rbAlsaPlayback(RBRawAudio* audio_buf, int num_frames, int channels)
{
    if(playback_buffer == NULL || playback_handle == NULL) {
        // Init must have failed -- playback is non-essential so don't fatal
        return;
    }

    for (int frame = 0; frame < num_frames; frame++) {
        for (int channel = 0; channel < channels; channel++) {
            playback_buffer[frame*channels+channel] = audio_buf->audio[frame][channel];
        }
    }

    snd_pcm_sframes_t frames;

    frames = snd_pcm_writei(playback_handle, playback_buffer, num_frames);
    if (frames < 0) {
        frames = snd_pcm_recover(playback_handle, frames, 0);
    }
    if (frames < 0) {
        rbError("snd_pcm_writei failed\n");
    }
}
