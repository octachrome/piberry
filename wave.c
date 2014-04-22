// Work in progress to send a tone to the pi headphone port
// Ideas taken from https://github.com/dwelch67/raspberrypi/tree/master/blinker01
// and http://crca.ucsd.edu/~msp/tmp/mmap-sinetest.c

#ifdef LINUX
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
static volatile void *peripheral_base;
#define PERIPHERAL_BASE peripheral_base
#endif
#include "pi.h"

#define PULSES_PER_SAMPLE   2048

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
        amp *= 0.9999;
    }
}

void delay() {
    int i;
    for (i = 0; i < 0x100000; i++) {
        dummy(i);
    }
}

void delay_short() {
    int i;
    for (i = 0; i < 150; i++) {
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
    // use PLLD @ 500MHz
    PWMCLK_CNTL = 0x5a000006;
    // enable clock (bit 4)
    PWMCLK_CNTL = 0x5a000016;
    // wait for busy flag to set
    while (!(PWMCLK_CNTL & (1 << 7)));
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

volatile int trigger;

void play() {
    int i = 0;
    while (1) {
        if (trigger) {
            trigger = 0;
            i = 0;
        }

        if (i < sampleCount) {
            // channel 0
            writeSample(sampleTable[i]);
            // channel 1
            writeSample(sampleTable[i]);
            i++;
        }
    }
}


__attribute__((interrupt("IRQ")))
void irq_handler() {
    trigger = 1;

    // Clear interrupt status (yes, by writing 1s to it)
    GPEDS0 = 0xffffffff;
}

void configure_gpio_irq() {
    // GPIO17 as input
    GPFSEL1 = GPFSEL1 & ~(7 << 21);

    // Pull-up on GPIO17
    GPPUD = 2;
    delay_short();
    GPPUDCLK0 = (1 << 17);
    delay_short();
    GPPUD = 0;
    GPPUDCLK0 = 0;

    // IRQ when GPIO17 is pulled low
    GPFEN0 = GPFEN0 | (1 << 17);

    // Enable IRQ49 = gpio_irq[0] = interrupts from GPIO pins 0-31
    INTEN2 = 1 << (49 - 32);
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
    trigger = 1;
    play();
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

void install_except_handler(int index, void* handler) {
    // The vectors start after the 8 ldr instructions
    unsigned int* base = (unsigned int*) (8 * 4);
    base[index] = (unsigned int) handler;
}

void notmain() {
    sampleTable = heap;
    install_except_handler(6, irq_handler);
    configure_gpio_irq();

    common();
}
