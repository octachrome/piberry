// Work in progress to send a tone to the pi headphone port
// Ideas taken from https://github.com/dwelch67/raspberrypi/tree/master/blinker01
// and http://crca.ucsd.edu/~msp/tmp/mmap-sinetest.c

#define PHYS_PERIPHERAL_BASE 0x20000000
#define PERIPHERAL_RANGE	 0x00300000

#ifdef LINUX
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
static volatile void *peripheral_base;
#define PERIPHERAL_BASE peripheral_base
#else
#define PERIPHERAL_BASE	PHYS_PERIPHERAL_BASE
#endif

#define ACCESS_PERI(offset) *((unsigned int volatile*)(PERIPHERAL_BASE + offset))
#define WORDSIZE	4

#define GPFSEL1		ACCESS_PERI(0x200004)
#define GPFSEL4		ACCESS_PERI(0x200010)
#define GPSET0		ACCESS_PERI(0x20001C)
#define GPCLR0		ACCESS_PERI(0x200028)

#define PWM_CONTROL ACCESS_PERI(0x20C000)
#define PWM_STATUS  ACCESS_PERI(0x20C004)
#define PWM0_RANGE  ACCESS_PERI(0x20C010)
#define PWM0_DATA   ACCESS_PERI(0x20C014)
#define PWM_FIFO    ACCESS_PERI(0x20C018)
#define PWM1_RANGE  ACCESS_PERI(0x20C020)
#define PWM1_DATA   ACCESS_PERI(0x20C024)

#define PWMCLK_CNTL ACCESS_PERI(0x1010A0)
#define PWMCLK_DIV  ACCESS_PERI(0x1010A4)

#define PULSES_PER_SAMPLE	2048

extern int dummy (unsigned int);

extern void *heap;
float *sampleTable;

long sampleCount = -1;

void generate_waveform(int freq, float amp, int secs, int sampleRate) {
	sampleCount = sampleRate * secs;
	int samplesPerWave = sampleRate / freq;
	int i;
	for (i = 0; i < sampleCount; i++) {
		int j = i % samplesPerWave;
		float progress = (float) j / samplesPerWave;

		if (progress < 0.5) {
			sampleTable[i] = amp * (progress * 4 - 1);
		} else {
			sampleTable[i] = amp * (3 - progress * 4);
		}
	}
}

void delay() {
	int i;
	for (i = 0; i < 0x100000; i++) {
		dummy(i);
	}
}

void blink() {
	// GPIO16 as output
	GPFSEL1 = (GPFSEL1 & ~(7 << 18)) | (1 << 18);

	int i = 3;
	while (i-- > 0) {
		GPSET0 = 1 << 16;
		delay();
		GPCLR0 = 1 << 16;
		delay();
	}
}

void configure_pwm_clock() {
	// disable pwm
	PWM_CONTROL = 0;
	// disable clock
	PWMCLK_CNTL = 0x5a000000 | (PWMCLK_CNTL & ~(1 << 4));
	// wait for busy flag to clear
	while (PWMCLK_CNTL & (1 << 7));
	// divide clock by 5.08626302083 to get to 2048*48KHz
	PWMCLK_DIV = 0x5a005058;
	// use PLLA @ 393.216MHz
	PWMCLK_CNTL = 0x5a000006;
	// enable clock (bit 4)
	PWMCLK_CNTL = 0x5a000016;
	// wait for busy flag to set
	while (!(PWMCLK_CNTL & (1 << 7)));

	blink();
}

void configure_pwm() {
	// map gpio40 to pwm0 out (4 = alt function 0)
	GPFSEL4 = (GPFSEL4 & ~(7)) | 4;
	// map gpio45 to pwm1 out (4 = alt function 0)
	GPFSEL4 = (GPFSEL4 & ~(7 << 15)) | (4 << 15);

	PWM0_RANGE = PULSES_PER_SAMPLE;
	PWM1_RANGE = PULSES_PER_SAMPLE;

	// enable pwm0, use fifo for pwm0, enable pwm1, use fifo for pwm1 
	PWM_CONTROL = 1 + (1 << 5) + (1 << 8) + (1 << 13);
}

void writeSample(float sample) {
	int data = PULSES_PER_SAMPLE * ((1.0 + sample) / 2);
	// wait for space in fifo
	while (PWM_STATUS & 1);
	// write data
	PWM_FIFO = data;
}

void play() {
	int i;
	for (i = 0; i < sampleCount; i++) {
		// channel 0
		writeSample(sampleTable[i]);
		// channel 1
		writeSample(sampleTable[i]);
	}
}

#ifdef LINUX
volatile void *map_memory(unsigned int base, unsigned int size)
{
	int fd;
	volatile void *map;

	/* open /dev/mem */
	if ((fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
		printf("can't open /dev/mem \n");
		exit(-1);
	}

	/* mmap GPIO */
	map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, base);

	close(fd); //No need to keep mem_fd open after mmap

	if (map == MAP_FAILED) {
		printf("mmap error %d\n", (int) map);//errno also set!
		exit(-1);
	}

	return map;
}
#endif

void common() {
	generate_waveform(440, 0.8, 1, 48000);
	configure_pwm_clock();
	configure_pwm();
	play();
	int i;
	for (i = 0; i < sampleCount; i++) {
		//printf("%f\n", sampleTable[i]);
	}
}

#ifdef LINUX
int main() {
	float y = (float) dummy(5);
	dummy((int) (y / 2.2));

	peripheral_base = map_memory(PHYS_PERIPHERAL_BASE, PERIPHERAL_RANGE);
	sampleTable = (float *) malloc(48000 * sizeof(float));
	common();
	return 0;
}
#endif

void notmain() {
	sampleTable = heap;
	common();
	while (1) blink();
}
