#include <stdio.h>

#include "audio.h"
#include "pi.h"

#define BLOCKS_PER_SEC (FRAME_RATE / BLOCK_FRAMES)

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

mod_handle_t patch;

void onevent(int event_type, int key)
{
    if (event_type == KEY_DOWN) {
        mod_trigger(patch, key);
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

    mod_handle_t kick = kick_create();
    mod_handle_t snare = snare_create();
    mod_handle_t mixed = add_create(kick, snare);

    patch = fm_create();

    audio_init();

    int j = 0;
    while (1) {
        if (j % BLOCKS_PER_SEC == 0) {
            mod_trigger(patch, 20 + j / BLOCKS_PER_SEC);
            mod_trigger(snare, 0);
        } else if (j % BLOCKS_PER_SEC == (BLOCKS_PER_SEC/2)) {
            mod_trigger(kick, 0);
        }
        j++;
        // kbd_scan();
        mod_newblock();
        float* block = mod_rdblock(mixed);
        audio_write(block);
    }

    audio_free();
}
