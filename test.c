#include "audio.h"
#include "pi.h"

#define BLOCKS_IN_HALF_SEC (FRAME_RATE / BLOCK_FRAMES / 2)
#define FREQ 440

#ifndef LINUX
extern int dummy(int);

void delay() {
    int i;
    for (i = 0; i < 0x100000; i++) {
        dummy(i);
    }
}

void delay_short() {
    int i;
    for (i = 0; i < 150; i++) {
        dummy(i);
    }
}

void configure_gpio_irq() {
    // GPIO17 as input
    GPFSEL1 = GPFSEL1 & ~(7 << 21);

    // Pull-up on GPIO17
    GPPUD = 2;
    delay_short();
    GPPUDCLK0 = (1 << 17);
    delay_short();
    GPPUD = 0;
    GPPUDCLK0 = 0;

    // IRQ when GPIO17 is pulled low
    GPFEN0 = GPFEN0 | (1 << 17);

    // Enable IRQ49 = gpio_irq[0] = interrupts from GPIO pins 0-31
    INTEN2 = 1 << (49 - 32);
}

void blink() {
    // GPIO16 as output
    GPFSEL1 = (GPFSEL1 & ~(7 << 18)) | (1 << 18);

    int i = 2;
    while (i-- > 0) {
        GPSET0 = 1 << 16;
        delay();
        GPCLR0 = 1 << 16;
        delay();
    }
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

volatile char trigger = 0;

#ifdef LINUX
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
    mod_handle_t expn = exp_create(env, 50, 1.2);
    mod_handle_t cosine = cos_create_vco(expn);
    mod_handle_t multiply = multiply_create(cosine, env);

    audio_init();

    int i = 0;
    while (1) {
        if (trigger || i++ % BLOCKS_IN_HALF_SEC == 0) {
            mod_trigger(env);
            mod_trigger(cosine);
            trigger = 0;
        }

        mod_newblock();
        float* block = mod_rdblock(multiply);
        audio_write(block);
    }

    audio_free();
}
