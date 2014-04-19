#include <math.h>
#include <stdio.h>

#include "audio.h"

// Two channels for one second
#define NSAMPLES SAMPLE_RATE * 2
#define FREQ 440
#define SAMP_PER_CYCLE (SAMPLE_RATE / FREQ)

#define FACT_3 (3*2)
#define FACT_5 (5*4*3*2)
#define FACT_7 (7*6*5*4*3*2)

#define TABLE_LEN 1200

sine() {
    float table[TABLE_LEN];

    int i;
    for (i = 0; i < TABLE_LEN / 4; i++) {
        float theta = 2 * M_PI * i / TABLE_LEN;
        float t3 = theta * theta * theta;
        float t5 = t3 * theta * theta;
        // float t7 = t5 * theta * theta;
        float s = theta - t3 / FACT_3 + t5 / FACT_5 /*- t7 / FACT_7*/;
        table[i] = s;
        table[i + TABLE_LEN / 2] = -s;
    }

    for (i = 0; i < TABLE_LEN / 4; i++) {
        table[TABLE_LEN / 2 - i - 1] = table[i];
        table[TABLE_LEN - i - 1] = -table[i];
    }

    short buf[NSAMPLES];

    for (i = 0; i < NSAMPLES; i += 2) {
        int phase = (i/2) % SAMP_PER_CYCLE;
        int idx = TABLE_LEN * phase / SAMP_PER_CYCLE;

        buf[i] = buf[i + 1] = (int) (table[idx] * 10000);

        if (i < 500) {
            printf("%d\n", buf[i]);
        }
    }

    audio_init();

    for (i = 0; i < NSAMPLES / 256; i++) {
        audio_queue(buf + i * 256, 128);
    }

    audio_free();
}

main() {
    sine();
}
