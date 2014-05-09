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

/*#ifdef LINUX
void main()
#else
void notmain()
#endif
{
#ifndef LINUX
    install_except_handler(6, irq_handler);
    configure_gpio_irq();
#endif

    mod_handle_t env = envelope_create(0.001, 0.2);
    mod_handle_t expn = exp_create(env, 40, 1.2);
    mod_handle_t cosine = cos_create_vco(expn);
    // mod_handle_t out = multiply_create(cosine, env);

    mod_handle_t keyval = value_create(0);
    mod_handle_t expn2 = exp_create(keyval, 220, 1.0/12);
    mod_handle_t out = cos_create_vco(expn2);

    audio_init();

    int i = 0, j = 0;
    while (1) {
        if (trigger || i++ % BLOCKS_IN_HALF_SEC == 0) {
            mod_trigger(env, 0);
            mod_trigger(cosine, 0);
            mod_trigger(keyval, j++);
            trigger = 0;
        }

        mod_newblock();
        float* block = mod_rdblock(out);
        audio_write(block);
    }

    audio_free();
}
*/

void onevent(int event_type, int key)
{
    if (event_type == KEY_UP) {
        printf("key up: %d\n", key);
    } else {
        printf("key down: %d\n", key);
    }
}

int main() {
    int banks[] = {10, 15, 14, 27, 17, 9, 22, 4, 11};
    int nbanks = 9;
    int inputs[] = {18, 23, 24, 25, 8, 7};
    int ninputs = 6;

    gpio_init();
    kbd_init(banks, nbanks, inputs, ninputs, onevent);

    while (1) {
        kbd_scan();
    }

    return 0;
}
