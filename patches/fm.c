#include "../audio.h"

typedef struct {
    mod_handle_t keyval;
    mod_handle_t cosine;
    mod_handle_t env;
    mod_handle_t sr;
} fm_data_t;

static void fm_ontrigger(mod_handle_t handle, void* d, float value)
{
    fm_data_t* data = (fm_data_t*) d;
    // Change the note being played
    mod_trigger(data->keyval, value);
    // Reset the phase of the waveform
    mod_trigger(data->cosine, value);
    // Trigger the envelope
    mod_trigger(data->env, value);
    // Trigger the switch-and-ramp
    mod_trigger(data->sr, value);
}

mod_handle_t fm_create()
{
    mod_handle_t keyval = value_create(20);
    mod_handle_t carrier_freq = exp_create(keyval, 65, 1.0/12);

    mod_handle_t modulator_freq = multiply_create(carrier_freq, value_create(5.2));
    mod_handle_t modulator = cos_create_vco(modulator_freq);

    mod_handle_t freq = add_create(carrier_freq, multiply_create(modulator, value_create(18)));
    mod_handle_t carrier = cos_create_vco(freq);

    mod_handle_t env = envelope_create(0.01, 1);
    mod_handle_t sig = multiply_create(carrier, env);
    mod_handle_t sr = switchramp_create(sig, 0.01);

    mod_handle_t handle = mod_create_patch(sr, fm_ontrigger, sizeof(fm_data_t));

    fm_data_t* data = (fm_data_t*) mod_data(handle);
    data->keyval = keyval;
    data->cosine = carrier;
    data->env = env;
    data->sr = sr;

    return handle;
}
