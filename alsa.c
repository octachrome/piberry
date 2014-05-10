/**
 * Audio wrapper for ALSA on Linux.
 *
 * Derived from http://equalarea.com/paul/alsa-audio.html
 * aplay -l to list all ALSA devices; modify DEVICE, e.g., hw:0,0
 */

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#include "audio.h"

#define DEVICE "hw:0,0"

static snd_pcm_t *playback_handle;

void audio_init()
{
    int err;
    snd_pcm_hw_params_t *hw_params;

    if ((err = snd_pcm_open (&playback_handle, DEVICE, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n", 
             DEVICE,
             snd_strerror (err));
        exit (1);
    }
       
    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
             snd_strerror (err));
        exit (1);
    }
             
    if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
             snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",
             snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",
             snd_strerror (err));
        exit (1);
    }

    int rate = FRAME_RATE;
    if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &rate, 0)) < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n",
             snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, 2)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n",
             snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
             snd_strerror (err));
        exit (1);
    }

    snd_pcm_hw_params_free (hw_params);

    if ((err = snd_pcm_prepare (playback_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
             snd_strerror (err));
        exit (1);
    }
}

void audio_write(float* block)
{
    short buffer[BLOCK_SIZE];
    int i;
    for (i = 0; i < BLOCK_SIZE; i++) {
        buffer[i] = (short) (block[i] * 32767);
    }
    int err;
    if ((err = snd_pcm_writei (playback_handle, buffer, BLOCK_FRAMES)) != BLOCK_FRAMES) {
        fprintf (stderr, "write to audio interface failed (%s)\n",
             snd_strerror (err));
        exit (1);
    }
}

void audio_free()
{
    snd_pcm_drain (playback_handle);
    snd_pcm_close (playback_handle);
}
