#include "audio.h"

#define RANDOM_PRIME            18223
#define PRIME_NEAR_MAX_INT      2147483647

typedef struct {
    int val;
} noise_data_t;

static void noise_fillblock(mod_handle_t handle, float* block, void* d)
{
    noise_data_t* data = d;
    int val = data->val;
    union {
        int i;
        float f;
    } u;
    int i;
    for (i = 0; i < BLOCK_SIZE; i += 2) {
        // Mask out the exponent, leaving the fraction and the sign, and add a biased exponent of -1. This gives a number between -1 and 1.
        u.i = (val & 0x807fffff) | ((-1 + 127) << 23);
        block[i] = block[i+1] = u.f;
        val = val * RANDOM_PRIME % PRIME_NEAR_MAX_INT;
    }
    data->val = val;
}

mod_handle_t noise_create()
{
    mod_handle_t handle = mod_create(noise_fillblock, 0, sizeof(noise_data_t));
    noise_data_t* data = mod_data(handle);
    data->val = 1;

    return handle;
}
