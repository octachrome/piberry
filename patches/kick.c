#include "../audio.h"

typedef struct {
    mod_handle_t cosine;
    mod_handle_t env;
} kick_data_t;

static void kick_ontrigger(mod_handle_t handle, void* d, float value)
{
    kick_data_t* data = (kick_data_t*) d;
    // Reset the phase of the waveform
    mod_trigger(data->cosine, value);
    // Trigger the envelope
    mod_trigger(data->env, value);
}

mod_handle_t kick_create()
{
    // Envelope controls both the amplitude and the pitch
    mod_handle_t env = envelope_create(0.001, 0.2);
    // Pitch of the oscillator
    mod_handle_t freq = exp_create(env, 40, 1.2);
    // The oscillator
    mod_handle_t cosine = cos_create_vco(freq);
    // Apply the envelope to the waveform
    mod_handle_t sig = multiply_create(cosine, env);

    mod_handle_t handle = mod_create_patch(sig, kick_ontrigger, sizeof(kick_data_t));

    kick_data_t* data = (kick_data_t*) mod_data(handle);
    data->cosine = cosine;
    data->env = env;

    return handle;
}
