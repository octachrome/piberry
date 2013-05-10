// Work in progress to send a tone to the pi headphone port
// Ideas taken from https://github.com/dwelch67/raspberrypi/tree/master/blinker01
// and http://crca.ucsd.edu/~msp/tmp/mmap-sinetest.c

#define WORDSIZE	4
#define GPFSEL1		0x20200004
#define GPSET0		0x2020001C
#define GPCLR0		0x20200028

#define PUT(addr, value) (*((unsigned int volatile *) addr) = (value))
#define GET(addr) (*((unsigned int volatile *) addr))

extern void dummy (unsigned int);

extern float sampleTable[];
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
	PUT(GPFSEL1, (GET(GPFSEL1) & ~(7 << 18)) | (1 << 18));

	while (1) {
		PUT(GPSET0, 1 << 16);
		delay();
		PUT(GPCLR0, 1 << 16);
		delay();
	}
}

void notmain() {
	generate_waveform(440, 1, 1, 48000);
	int i;
	for (i = 0; i < sampleCount; i++) {
		//printf("%f\n", sampleTable[i]);
	}

	blink();
}
