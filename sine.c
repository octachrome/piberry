#include <math.h>

#include "audio.h"

#define FACT_4 (4*3*2)
#define FACT_6 (6*5*4*3*2)

#define TABLE_LEN 1200

char table_ready = 0;
float table[TABLE_LEN];

static void create_table()
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
    mod_handle_t freq_in;
    float freq;
    float phase;
} cos_data_t;

static void cos_fillblock(mod_handle_t handle, float* block, void* d)
{
    cos_data_t* data = (cos_data_t*) d;
    float freq = data->freq;
    float phase = data->phase;

    float* freq_in;
    if (freq == 0) {
        freq_in = mod_rdblock(data->freq_in);
    }

    int i;
    for (i = 0; i < BLOCK_FRAMES; i++) {
        int idx = (int) (phase * TABLE_LEN);
        block[i * 2] = block[i * 2 + 1] = table[idx];

        if (freq > 0) {
            phase += freq / FRAME_RATE;
        } else {
            phase += freq_in[i] / FRAME_RATE;
        }
        if (phase >= 1) phase -= 1;
    }

    data->phase = phase;
}

static mod_handle_t cos_create(float freq, mod_handle_t freq_in)
{
    if (!table_ready) {
        create_table();
    }

    mod_handle_t handle = mod_create(cos_fillblock, 0, sizeof(cos_data_t));

    cos_data_t* data = (cos_data_t*) mod_data(handle);

    if (freq > 0) {
        data->freq = freq;
        data->freq_in = -1;
    } else {
        data->freq_in = freq_in;
        data->freq = 0;
    }
    data->phase = 0;

    return handle;
}

mod_handle_t cos_create_fixed(float freq)
{
    return cos_create(freq, -1);
}

mod_handle_t cos_create_vco(mod_handle_t freq_in)
{
    return cos_create(0, freq_in);
}

#define BLOCKS_IN_ONE_SEC (FRAME_RATE / BLOCK_FRAMES)
#define FREQ 440

main()
{
    mod_handle_t env = envelope_create(0.0005, 0.14);
    mod_handle_t expn = exp_create(env, 40, .8);
    mod_handle_t cosine = cos_create_vco(expn);
    mod_handle_t multiply = multiply_create(cosine, env);

    audio_init();

    int i;
    for (i = 0; i < BLOCKS_IN_ONE_SEC; i++) {
        mod_newblock();
        float* block = mod_rdblock(multiply);
        audio_write(block);
    }

    audio_free();
}
