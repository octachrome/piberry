#include <math.h>

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
		block[i] = a * pow(2.0, c * in[i]);
	}
}

mod_handle_t exp_create(mod_handle_t in, float a, float c)
{
	mod_handle_t handle = mod_create(exp_fillblock, 0, sizeof(exp_data_t));
	exp_data_t* data = mod_data(handle);
	data->in = in;
	data->a = a;
	data->c = c;

	return handle;
}
