#include <stdio.h>
#include "audio.h"

// int banks[] = {15, 4, 11, 17, 10, 22, 27, 9, 14};
int banks[] = {10, 15, 14, 27, 17, 9, 22, 4, 11};
int nbanks = 9;
int inputs[] = {18, 23, 24, 25, 8, 7};
int ninputs = 6;

int main() {
    gpio_init();

    int b, i;
    for (b = 0; b < nbanks; b++) {
        gpio_config(banks[b], GPIO_OUT);
        gpio_set(banks[b]);
    }

    for (i = 0; i < ninputs; i++) {
        gpio_config(inputs[i], GPIO_IN);
    }
    gpio_pullups(inputs, ninputs);

    while (1) {
        for (b = 0; b < nbanks; b++) {
            int bank = banks[b];
            gpio_clear(bank);
            delay(100000);

            for (i = 0; i < ninputs; i++) {
                int input = inputs[i];

                if (gpio_level(input) == 0) {
                    printf("key %d %d\n", b, i);
                }
            }

            gpio_set(bank);
        }
    }

    return 0;
}
