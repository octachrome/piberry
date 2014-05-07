#include <stdio.h>
#include "audio.h"

int* banks;
int nbanks;
int* inputs;
int ninputs;

// Hopefully this is larger than nbanks
int keymap[20];

void kbd_init(int* banks_, int nbanks_, int* inputs_, int ninputs_)
{
    banks = banks_;
    nbanks = nbanks_;
    inputs = inputs_;
    ninputs = ninputs_;

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
    for (b = 0; b < nbanks; b++) {
        int bank = banks[b];
        gpio_clear(bank);
        delay(100000);

        for (i = 0; i < ninputs; i++) {
            int input = inputs[i];

            if (gpio_level(input) == 0) {
                // printf("key %d %d\n", b, i);
            }
        }

        gpio_set(bank);
    }
}

/*int main() {
    int banks[] = {10, 15, 14, 27, 17, 9, 22, 4, 11};
    int nbanks = 9;
    int inputs[] = {18, 23, 24, 25, 8, 7};
    int ninputs = 6;

    gpio_init();
    kbd_init(banks, nbanks, inputs, ninputs);

    while (1) {
        kbd_scan();
    }

    return 0;
}
*/

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
