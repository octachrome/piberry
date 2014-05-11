#include "audio.h"

struct {
    int* banks;
    int nbanks;
    int* inputs;
    int ninputs;
    unsigned int keystates[4]; // space for 128 midi keys
    kbd_onevent_t onevent;
} data;


void kbd_init(int* banks, int nbanks, int* inputs, int ninputs, kbd_onevent_t onevent)
{
    data.banks = banks;
    data.nbanks = nbanks;
    data.inputs = inputs;
    data.ninputs = ninputs;
    data.onevent = onevent;

    int b, i;
    for (b = 0; b < nbanks; b++) {
        gpio_config(banks[b], GPIO_OUT);
        gpio_set(banks[b]);
    }

    for (i = 0; i < ninputs; i++) {
        gpio_config(inputs[i], GPIO_IN);
    }
    gpio_pullups(inputs, ninputs);
}


void kbd_scan() {
    int b, i;
    for (b = 0; b < data.nbanks; b++) {
        int bank = data.banks[b];
        gpio_clear(bank);
        delay(100);

        for (i = 0; i < data.ninputs; i++) {
            int input = data.inputs[i];
            int key = b * data.ninputs + i;

            int byte = key / 32;
            int bit = key % 32;
            unsigned int state = data.keystates[byte] & (1 << bit);

            if (gpio_level(input) == 0) {
                if (!state) {
                    data.onevent(KEY_DOWN, key);
                    data.keystates[byte] |= (1 << bit);
                }
            } else {
                if (state) {
                    data.onevent(KEY_UP, key);
                    data.keystates[byte] &= ~(1 << bit);
                }
            }
        }

        gpio_set(bank);
    }
}

void value_fillblock(mod_handle_t handle, float* block, void* d)
{
    float value = *((float*) d);

    int i;
    for (i = 0; i < BLOCK_FRAMES; i++) {
        block[i * 2] = block[i * 2 + 1] = value;
    }
}

void value_ontrigger(mod_handle_t handle, void* d, float value)
{
    float* data = (float*) d;
    *data = value;
}

mod_handle_t value_create(float value)
{
    mod_handle_t handle = mod_create(value_fillblock, value_ontrigger, sizeof(float));

    float* data = mod_data(handle);
    *data = value;
    return handle;
}
