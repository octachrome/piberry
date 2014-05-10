#include <stdio.h>
#include "audio.h"
#include "pi.h"

#define BLOCKS_IN_HALF_SEC (FRAME_RATE / BLOCK_FRAMES / 2)

volatile char trigger = 0;

#ifndef LINUX
void configure_gpio_irq() {
    gpio_config(17, GPIO_IN);
    gpio_pullup(17);

    // IRQ when GPIO17 is pulled low
    GPFEN0 = GPFEN0 | (1 << 17);

    // Enable IRQ49 = gpio_irq[0] = interrupts from GPIO pins 0-31
    INTEN2 = 1 << (49 - 32);
}

__attribute__((interrupt("IRQ")))
void irq_handler() {
    trigger = 1;

    // Clear interrupt status (yes, by writing 1s to it)
    GPEDS0 = 0xffffffff;
}

void install_except_handler(int index, void* handler) {
    // The vectors start after the 8 ldr instructions
    unsigned int* base = (unsigned int*) (8 * 4);
    base[index] = (unsigned int) handler;
}
#endif

mod_handle_t env;
mod_handle_t cosine;
mod_handle_t keyval;

void onevent(int event_type, int key)
{
    if (event_type == KEY_UP) {
        printf("key up: %d\n", key);
    } else {
        printf("key down: %d\n", key);
        mod_trigger(keyval, key);
        mod_trigger(env, 0);
        mod_trigger(cosine, 0);
    }
}

#ifdef LINUX
void main()
#else
void notmain()
#endif
{
    int banks[] = {10, 15, 14, 27, 17, 9, 22, 4, 11};
    int nbanks = 9;
    int inputs[] = {18, 23, 24, 25, 8, 7};
    int ninputs = 6;

    gpio_init();
    kbd_init(banks, nbanks, inputs, ninputs, onevent);

    // env = envelope_create(0.001, 0.2);
    // mod_handle_t expn = exp_create(env, 40, 1.2);

    keyval = value_create(0);
    env = envelope_create(0.01, 1);
    mod_handle_t expn = exp_create(keyval, 65, 1.0/12);

    cosine = cos_create_vco(expn);
    mod_handle_t out = multiply_create(cosine, env);

    audio_init();


    while (1) {
        kbd_scan();
        mod_newblock();
        float* block = mod_rdblock(out);
        audio_write(block);
    }

    audio_free();
}
