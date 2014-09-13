#include "audio.h"

#define RANDOM_PRIME            18223
#define PRIME_NEAR_MAX_INT      2147483647
#define PRIME_NEAR_MAX_SHORT    32749

typedef struct {
    int val;
} noise_data_t;

static void noise_fillblock(mod_handle_t handle, float* block, void* d)
{
    noise_data_t* data = d;
    int val = data->val;
    int i;
    for (i = 0; i < BLOCK_SIZE; i += 2) {
        block[i] = block[i+1] = (float) (val % PRIME_NEAR_MAX_SHORT) / PRIME_NEAR_MAX_SHORT;
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
