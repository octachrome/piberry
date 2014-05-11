#include "../audio.h"

float block[BLOCK_SIZE];

float* mod_rdblock(mod_handle_t handle)
{
    return block;
}

mod_handle_t mod_create(mod_fillblock_t fillblock, mod_ontrigger_t ontrigger, int bytes)
{
    return 0;
}

void* mod_data(mod_handle_t handle)
{
    return 0;
}
