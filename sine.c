#include <math.h>

#include "audio.h"

#define FACT_4 (4*3*2)
#define FACT_6 (6*5*4*3*2)

#define TABLE_LEN 1200

char table_ready = 0;
float table[TABLE_LEN];

void create_table()
{
    int i;
    for (i = 0; i < TABLE_LEN / 4; i++) {
        float theta = 2 * M_PI * i / TABLE_LEN;
        float t2 = theta * theta;
        float t4 = t2 * t2;
        float t6 = t4 * t2;
        float s = theta - t2 / 2 + t4 / FACT_4 - t6 / FACT_6;
        table[i] = s;
        table[i + TABLE_LEN / 2] = -s;
    }

    for (i = 0; i < TABLE_LEN / 4; i++) {
        table[TABLE_LEN / 2 - i - 1] = table[i];
        table[TABLE_LEN - i - 1] = -table[i];
    }

    table_ready = 1;
}

typedef struct {
    unsigned int samplesPerCycle;
    unsigned int samplePos;
} cos_data_t;

void cos_fillblock(mod_handle_t handle, float* block, void* d)
{
    cos_data_t* data = (cos_data_t*) d;
    int samplePos = data->samplePos;
    int samplesPerCycle = data->samplesPerCycle;

    int i;
    for (i = 0; i < BLOCK_FRAMES; i++) {
        int phase = samplePos % samplesPerCycle;
        int idx = TABLE_LEN * phase / samplesPerCycle;

        block[i * 2] = block[i * 2 + 1] = table[idx];
        samplePos++;
    }

    data->samplePos = samplePos;
}

mod_handle_t cos_create(float freq)
{
    if (!table_ready) {
        create_table();
    }

    mod_handle_t handle = mod_create(cos_fillblock, 0, sizeof(cos_data_t));

    cos_data_t* data = (cos_data_t*) mod_data(handle);

    data->samplesPerCycle = FRAME_RATE / freq;
    data->samplePos = 0;

    return handle;
}

#define BLOCKS_IN_ONE_SEC (FRAME_RATE / BLOCK_FRAMES)
#define FREQ 440

main()
{
    mod_handle_t cosine = cos_create(FREQ);
    mod_handle_t env = envelope_create(0.01, 0.8);
    mod_handle_t multiply = multiply_create(cosine, env);

    audio_init();

    int i;
    for (i = 0; i < BLOCKS_IN_ONE_SEC; i++) {
        float* block = mod_rdblock(multiply);
        audio_write(block);
    }

    audio_free();
}
