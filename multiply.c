#include "audio.h"

typedef struct {
    mod_handle_t op1;
    mod_handle_t op2;
} multiply_data_t;

void multiply_fillblock(mod_handle_t handle, float* block, void* d)
{
    multiply_data_t* data = (multiply_data_t*) d;
    float* block1 = mod_rdblock(data->op1);
    float* block2 = mod_rdblock(data->op2);
    int i;
    for (i = 0; i < BLOCK_SIZE; i++) {
        block[i] = block1[i] * block2[i];
    }
}

mod_handle_t multiply_create(mod_handle_t op1, mod_handle_t op2)
{
    mod_handle_t handle = mod_create(multiply_fillblock, 0, sizeof(multiply_data_t));
    multiply_data_t* data = mod_data(handle);
    data->op1 = op1;
    data->op2 = op2;

    return handle;
}

#define LN2 0.693147180559945309417

#define FACT_3 (3*2)
#define FACT_4 (4*3*2)
#define FACT_5 (5*4*3*2)
#define FACT_6 (6*5*4*3*2)

static char exp_table_ready = 0;
static float exp_table[TABLE_LEN];

void create_exp_table()
{
    int i;
    for (i = 0; i < TABLE_LEN; i++) {
        float f = LN2 * i / TABLE_LEN;
        float f2 = f * f;
        float f3 = f2 * f;
        float f4 = f3 * f;
        float f5 = f4 * f;
        float f6 = f5 * f;
        exp_table[i] = 1 + f + f2 / 2 + f3 / FACT_3 + f4 / FACT_4 + f5 / FACT_5 + f6 / FACT_6;
    }
    exp_table_ready = 1;
}

float pow2(float ex)
{
    int i = (int) ex;
    if (ex < 0) {
        float base = 1.0 / (1 << -i);
        float fract = i - ex;
        if (fract > 0) {
            float x = table_lookup(exp_table, fract, 2);
            base /= x;
        }
        return base;
    } else {
        float base = 1 << i;
        float fract = ex - i;
        if (fract > 0) {
            float x = table_lookup(exp_table, fract, 2);
            base *= x;
        }
        return base;
    }
}

typedef struct {
    mod_handle_t in;
    float a;
    float c;
} exp_data_t;

void exp_fillblock(mod_handle_t handle, float* block, void* d)
{
    exp_data_t* data = (exp_data_t*) d;
    float* in = mod_rdblock(data->in);
    float a = data->a;
    float c = data->c;
    int i;
    for (i = 0; i < BLOCK_SIZE; i++) {
        float exponent = c * in[i];
        block[i] = a * pow2(exponent);
    }
}

/**
 * Calculates a * 2^(c * in)
 */
mod_handle_t exp_create(mod_handle_t in, float a, float c)
{
    if (!exp_table_ready) {
        create_exp_table();
    }
    mod_handle_t handle = mod_create(exp_fillblock, 0, sizeof(exp_data_t));
    exp_data_t* data = mod_data(handle);
    data->in = in;
    data->a = a;
    data->c = c;

    return handle;
}
