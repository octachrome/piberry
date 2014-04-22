#include <stdio.h>
#include "audio.h"

typedef struct {
    unsigned int attack;
    unsigned int decay;
    unsigned int samplePos;
} envelope_data_t;

void envelope_fillblock(mod_handle_t handle, float* block, void* d)
{
    envelope_data_t* data = (envelope_data_t*) d;
    unsigned int attack = data->attack;
    unsigned int decay = data->decay;
    unsigned int samplePos = data->samplePos;

    int i;
    for (i = 0; i < BLOCK_FRAMES; i++) {
        float value;
        if (samplePos < attack) {
            value = ((float) samplePos) / attack;
            samplePos++;
        } else if (samplePos < attack + decay) {
            value = 1.0 - ((float) samplePos - attack) / decay;
            samplePos++;
        } else {
            value = 0;
        }
        block[i * 2] = block[i * 2 + 1] = value;
    }

    data->samplePos = samplePos;
}

void envelope_ontrigger(mod_handle_t handle, void* d)
{
    envelope_data_t* data = (envelope_data_t*) d;
    data->samplePos = 0;
}

mod_handle_t envelope_create(float attack, float decay)
{
    mod_handle_t handle = mod_create(envelope_fillblock, envelope_ontrigger, sizeof(envelope_data_t));

    envelope_data_t* data = mod_data(handle);
    data->attack = attack * FRAME_RATE;
    data->decay = decay * FRAME_RATE;
    data->samplePos = 0;
    return handle;
}
