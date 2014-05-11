/**
 * Dumps the audio stream to a raw file.
 */
#include <stdio.h>
#include <stdlib.h>

#include "audio.h"

FILE* file;

void audio_init()
{
    file = fopen("audio.raw", "w");
    if (file == 0) {
        printf("Failed to open audio.raw\n");
        exit(1);
    }
}

void audio_write(float* block)
{
    short buffer[BLOCK_SIZE];
    int i;
    for (i = 0; i < BLOCK_SIZE; i++) {
        buffer[i] = (short) (block[i] * 32767);
    }
    short* p = buffer;
    int remaining = BLOCK_SIZE;
    while (remaining > 0) {
        int written = fwrite(p, sizeof(short), remaining, file);
        p += written;
        remaining -= written;
    }
}

void audio_free()
{
    fclose(file);
}
