#define FRAME_RATE 48000                // samples per second, per channel
#define BLOCK_FRAMES 512                // frames per block
#define BLOCK_SIZE (BLOCK_FRAMES * 2)   // in words

void audio_init();
void audio_free();
void audio_write(float* block);

typedef int mod_handle_t;
typedef void (*mod_fillblock_t)(mod_handle_t handle, float* block, void* data);
typedef void (*mod_ontrigger_t)(mod_handle_t handle, void* data, float value);

mod_handle_t mod_create(mod_fillblock_t fillblock, mod_ontrigger_t ontrigger, int bytes);
mod_handle_t mod_create_patch(mod_handle_t proxy_handle, mod_ontrigger_t ontrigger, int bytes);
void* mod_data(mod_handle_t handle);
float* mod_rdblock(mod_handle_t handle);
void mod_trigger(mod_handle_t handle, float value);
void mod_newblock();

// A fixed-frequency cosine generator
mod_handle_t cos_create_fixed(float freq);
// A variable-frequency cosine generator
mod_handle_t cos_create_vco(mod_handle_t freq_in);
// A simple linear attack/decay envelope generator. Attack and decay times are in seconds.
mod_handle_t envelope_create(float attack, float decay);
// Samples the audio stream, and when triggered, linearly blends from the last sample to the next block.
mod_handle_t switchramp_create(mod_handle_t in, float time);
// Adds two input streams.
mod_handle_t add_create(mod_handle_t op1, mod_handle_t op2);
// Multiplies two input streams.
mod_handle_t multiply_create(mod_handle_t op1, mod_handle_t op2);
// Calculates a * 2 ^ (b * input).
mod_handle_t exp_create(mod_handle_t in, float a, float c);
// Continually repeats the value from the last trigger.
mod_handle_t value_create(float value);

// A really simple kick drum synth
mod_handle_t simple_create();
// A simple cosine synth
mod_handle_t simple_create();

void gpio_init();
int gpio_level(int gpio);
void gpio_set(int gpio);
void gpio_clear(int gpio);
void gpio_config(int gpio, int config);
void gpio_pullups(int* gpios, int count);
void gpio_pullup(int gpio);

#define GPIO_IN  0
#define GPIO_OUT 1

typedef void (*kbd_onevent_t)(int event_type, int key);
void kbd_init(int* banks, int nbanks, int* inputs, int ninputs, kbd_onevent_t onevent);
void kbd_scan();

#define KEY_UP   0
#define KEY_DOWN 1

#define TABLE_LEN 1200

static inline float table_lookup(float* table, float fidx, float max)
{
    float idx = fidx * TABLE_LEN;
    int idx_low = (int) idx;
    int idx_up = idx_low + 1;
    float idx_fract = idx - idx_low;
    float lower = table[idx_low];
    float upper = idx_up < TABLE_LEN ? table[idx_up] : max;
    return lower * (1 - idx_fract) + upper * idx_fract;
}

#ifdef __arm__
extern int __divsi3(int i, int j);
#define WORK __divsi3(12345, 321)
#else
extern int __divti3(int i, int j);
#define WORK __divti3(12345, 321)
#endif

static inline void delay(int cycles)
{
    int i;
    for (i = 0; i < cycles; i++) {
        WORK;
    }
}

static inline void delay_short()
{
    delay(150);
}

static inline void blink()
{
    gpio_config(16, GPIO_OUT);

    int i = 2;
    while (i-- > 0) {
        gpio_set(16);
        delay(1000000);
        gpio_clear(16);
        delay(1000000);
    }
}
