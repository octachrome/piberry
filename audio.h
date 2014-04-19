#define SAMPLE_RATE 48000

void audio_init();
void audio_free();
void audio_queue(void* buf, int len);
