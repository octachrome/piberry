#include <stdlib.h>

#include "audio.h"

typedef struct {
    mod_fillblock_t fillblock;
    mod_ontrigger_t ontrigger;
    float block[BLOCK_SIZE];
} mod_data_t;

static mod_data_t* modules[10];

static mod_handle_t next_handle = 0;

mod_handle_t mod_create(mod_fillblock_t fillblock, mod_ontrigger_t ontrigger, int bytes)
{
    mod_handle_t handle = next_handle++;

    mod_data_t* data = modules[handle] = malloc(sizeof(mod_data_t) + bytes);
    data->fillblock = fillblock;
    data->ontrigger = ontrigger;

    return handle;
}

void* mod_data(mod_handle_t handle)
{
    char* data = (char*) modules[handle];
    return data + sizeof(mod_data_t);
}

float* mod_rdblock(mod_handle_t handle)
{
    mod_data_t* data = modules[handle];
    data->fillblock(handle, data->block, ((char*) data) + sizeof(mod_data_t));
    return data->block;
}
