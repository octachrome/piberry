#include "../audio.h"

typedef struct {
    mod_handle_t env;
} snare_data_t;

static void snare_ontrigger(mod_handle_t handle, void* d, float value)
{
    snare_data_t* data = (snare_data_t*) d;
    // Trigger the envelope
    mod_trigger(data->env, value);
}

mod_handle_t snare_create()
{
    // Envelope controls the amplitude
    mod_handle_t env = envelope_create(0.001, 0.2);
    // The oscillator
    mod_handle_t noise = noise_create();
    // Apply the envelope to the waveform
    mod_handle_t sig = multiply_create(noise, env);

    mod_handle_t handle = mod_create_patch(sig, snare_ontrigger, sizeof(snare_data_t));

    snare_data_t* data = (snare_data_t*) mod_data(handle);
    data->env = env;

    return handle;
}
