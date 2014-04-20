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
void mod_newblock();
