#include "audio.h"
#include "pi.h"

#define PULSES_PER_SAMPLE	2048

/**
 * Audio wrapper for PWM on baremetal Pi.
 */

static void configure_pwm_clock() {
	// disable pwm
	PWM_CONTROL = 0;
	// disable clock
	PWMCLK_CNTL = 0x5a000000 | (PWMCLK_CNTL & ~(1 << 4));
	// wait for busy flag to clear
	while (PWMCLK_CNTL & (1 << 7));
	// divide clock by 5.08626302083 to get to 2048*48KHz
	PWMCLK_DIV = 0x5a005058;
	// use PLLD @ 500MHz
	PWMCLK_CNTL = 0x5a000006;
	// enable clock (bit 4)
	PWMCLK_CNTL = 0x5a000016;
	// wait for busy flag to set
	while (!(PWMCLK_CNTL & (1 << 7)));
}

static void configure_pwm() {
	// map gpio40 to pwm0 out (4 = alt function 0)
	GPFSEL4 = (GPFSEL4 & ~(7)) | 4;
	// map gpio45 to pwm1 out (4 = alt function 0)
	GPFSEL4 = (GPFSEL4 & ~(7 << 15)) | (4 << 15);
	PWM0_RANGE = PULSES_PER_SAMPLE;
	PWM1_RANGE = PULSES_PER_SAMPLE;

	// enable pwm0, use fifo for pwm0, enable pwm1, use fifo for pwm1 
	PWM_CONTROL = 1 + (1 << 5) + (1 << 8) + (1 << 13);
}

void audio_init()
{
	configure_pwm_clock();
	configure_pwm();
}

void audio_write(float* block)
{
	int i;
	for (i = 0; i < BLOCK_SIZE; i++) {
		int data = PULSES_PER_SAMPLE * ((1 + block[i]) / 2);
		// wait for space in fifo
		while (PWM_STATUS & 1);
		// write data
		PWM_FIFO = data;
	}
}

void audio_free()
{
}
