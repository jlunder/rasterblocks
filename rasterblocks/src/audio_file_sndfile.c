#ifdef RB_USE_SNDFILE_INPUT

#include "audio_file.h"

#include <sndfile.h>
#include <stdio.h>


SNDFILE* g_rbAudioInputFile;
int g_rbAudioChannels;


bool rbAudioFileInitialize(char const * filename)
{
    SF_INFO  sfinfo;

    g_rbAudioInputFile = sf_open(filename, SFM_READ, &sfinfo);
    if(g_rbAudioInputFile == NULL) {
        rbFatal("Not able to open input file %s.\n", filename);
        return false;
    }
    g_rbAudioChannels = sfinfo.channels;
    
    return true;
}


void rbAudioFileShutdown(void)
{
    if(g_rbAudioInputFile != NULL) {
        sf_close(g_rbAudioInputFile);
        g_rbAudioInputFile = NULL;
    }
}


void rbAudioFileReadLooping(RBRawAudio * pAudio)
{
    float buffer[g_rbAudioChannels * RB_AUDIO_FRAMES_PER_VIDEO_FRAME];
    int readCount;

    memset(pAudio->audio, 0, sizeof pAudio->audio);   
    readCount = sf_readf_float(g_rbAudioInputFile, buffer,
        RB_AUDIO_FRAMES_PER_VIDEO_FRAME);
    if(readCount < RB_AUDIO_FRAMES_PER_VIDEO_FRAME && readCount >= 0) {
        sf_seek(g_rbAudioInputFile,0,SEEK_SET);
        sf_readf_float(g_rbAudioInputFile,
            buffer + readCount * RB_AUDIO_CHANNELS,
            RB_AUDIO_FRAMES_PER_VIDEO_FRAME - readCount);
    }
    rbAssert(g_rbAudioChannels > 0);
    if(g_rbAudioChannels >= RB_AUDIO_CHANNELS) {
        for(size_t i = 0; i < RB_AUDIO_FRAMES_PER_VIDEO_FRAME; i++) {
            for (size_t j = 0; j < RB_AUDIO_CHANNELS; j++) {
                pAudio->audio[i][j] = buffer[i * g_rbAudioChannels + j];
            }
        }
    }
    else {
        for(size_t i = 0; i < RB_AUDIO_FRAMES_PER_VIDEO_FRAME; i++) {
            for (size_t j = 0; j < RB_AUDIO_CHANNELS; j++) {
                pAudio->audio[i][j] = buffer[i * g_rbAudioChannels + 0];
            }
        }
    }
}


#endif