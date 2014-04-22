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
#endif

void blink() {
#ifndef LINUX
    // GPIO16 as output
    GPFSEL1 = (GPFSEL1 & ~(7 << 18)) | (1 << 18);

    int i = 2;
    while (i-- > 0) {
        GPSET0 = 1 << 16;
        delay();
        GPCLR0 = 1 << 16;
        delay();
    }
#endif
}

#ifdef LINUX
void main()
#else
void notmain()
#endif
{
    mod_handle_t env = envelope_create(0.001, 0.2);
    mod_handle_t expn = exp_create(env, 50, 1.2);
    mod_handle_t cosine = cos_create_vco(expn);
    mod_handle_t multiply = multiply_create(cosine, env);

    audio_init();

    int i = 0;
    while (1) {
        if (i++ % BLOCKS_IN_HALF_SEC == 0) {
            mod_trigger(env);
            mod_trigger(cosine);
        }

        mod_newblock();
        float* block = mod_rdblock(multiply);
        audio_write(block);
    }

    audio_free();
}
