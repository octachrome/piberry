#define FRAME_RATE 48000				// samples per second, per channel
#define BLOCK_FRAMES 128				// frames per block
#define BLOCK_SIZE (BLOCK_FRAMES * 2)	// in words

void audio_init();
void audio_free();
void audio_write(float* block);

typedef int mod_handle_t;
typedef void (*mod_fillblock_t)(mod_handle_t handle, float* block, void* data);
typedef void (*mod_ontrigger_t)(mod_handle_t handle, void* data);

mod_handle_t mod_create(mod_fillblock_t fillblock, mod_ontrigger_t ontrigger, int bytes);
void* mod_data(mod_handle_t handle);
float* mod_rdblock(mod_handle_t handle);
void mod_trigger(mod_handle_t handle);
void mod_newblock();

mod_handle_t cos_create_fixed(float freq);
mod_handle_t cos_create_vco(mod_handle_t freq_in);
mod_handle_t envelope_create(float attack, float decay);
mod_handle_t multiply_create(mod_handle_t op1, mod_handle_t op2);
mod_handle_t exp_create(mod_handle_t in, float a, float c);

#define TABLE_LEN 1200

static inline float table_lookup(float* table, float fidx, float max)
{
    float idx = fidx * TABLE_LEN;
    int idx_low = (int) idx;
    int idx_up = idx_low + 1;
    float idx_fract = idx - idx_low;
    float lower = table[idx_low];
    float upper = idx_up < TABLE_LEN ? table[idx_up] : max;
    return (lower * (1 - idx_fract) + upper * idx_fract) / 2;
}
