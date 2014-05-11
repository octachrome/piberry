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

void envelope_ontrigger(mod_handle_t handle, void* d, float value)
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

typedef struct {
    mod_handle_t in;
    int triggered;
    unsigned int time;
    float leftSample;
    float rightSample;
} switchramp_data_t;

void switchramp_fillblock(mod_handle_t handle, float* block, void* d)
{
    switchramp_data_t* data = (switchramp_data_t*) d;
    float* in_block = mod_rdblock(data->in);
    unsigned int time = data->time;

    float leftRamp = 0;
    float rightRamp = 0;
    float leftDelta = 0;
    float rightDelta = 0;
    if (data->triggered) {
        leftRamp = data->leftSample - in_block[0];
        rightRamp = data->rightSample -  in_block[1];
        leftDelta = leftRamp / data->time;
        rightDelta = rightRamp / data->time;
        data->triggered = 0;
    }

    int i;
    for (i = 0; i < BLOCK_FRAMES; i++) {
        float left = in_block[i * 2];
        float right = in_block[i * 2 + 1];
        if (i < time) {
            left += leftRamp;
            right += rightRamp;
            leftRamp -= leftDelta;
            rightRamp -= rightDelta;
        }
        block[i * 2] = left;
        block[i * 2 + 1] = right;
    }
    data->leftSample = block[BLOCK_SIZE - 2];
    data->rightSample = block[BLOCK_SIZE - 1];
}

void switchramp_ontrigger(mod_handle_t handle, void* d, float value)
{
    switchramp_data_t* data = (switchramp_data_t*) d;
    data->triggered = 1;
}

mod_handle_t switchramp_create(mod_handle_t in, float time)
{
    mod_handle_t handle = mod_create(switchramp_fillblock, switchramp_ontrigger, sizeof(switchramp_data_t));

    switchramp_data_t* data = mod_data(handle);
    data->in = in;
    data->triggered = 0;
    data->time = time * FRAME_RATE;
    data->leftSample = 0;
    data->rightSample = 0;

    return handle;
}
