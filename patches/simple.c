#include "../audio.h"

typedef struct {
    mod_handle_t keyval;
    mod_handle_t cosine;
    mod_handle_t env;
    mod_handle_t sr;
} simple_data_t;

static void simple_ontrigger(mod_handle_t handle, void* d, float value)
{
    simple_data_t* data = (simple_data_t*) d;
    // Change the note being played
    mod_trigger(data->keyval, value);
    // Reset the phase of the waveform
    mod_trigger(data->cosine, value);
    // Trigger the envelope
    mod_trigger(data->env, value);
    // Trigger the switch-and-ramp
    mod_trigger(data->sr, value);
}

mod_handle_t simple_create()
{
    // Stores the last key pressed
    mod_handle_t keyval = value_create(20);
    // Convert the midi key value into a frequency
    mod_handle_t freq = exp_create(keyval, 65, 1.0/12);
    // The basic waveform
    mod_handle_t cosine = cos_create_vco(freq);
    // A simple envelope
    mod_handle_t env = envelope_create(0.01, 1);
    // Apply the envelope to the waveform
    mod_handle_t sig = multiply_create(cosine, env);
    // Switch-and-ramp to avoid clicks
    mod_handle_t sr = switchramp_create(sig, 0.01);

    mod_handle_t handle = mod_create_patch(sr, simple_ontrigger, sizeof(simple_data_t));

    simple_data_t* data = (simple_data_t*) mod_data(handle);
    data->keyval = keyval;
    data->cosine = cosine;
    data->env = env;
    data->sr = sr;

    return handle;
}
