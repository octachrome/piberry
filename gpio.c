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
#include "audio.h"

int gpio_level(int gpio)
{
#ifdef __arm__
    if (gpio < 32) {
        return GPLEV0 & (1 << gpio);
    } else {
        return GPLEV1 & (1 << (gpio - 32));
    }
#else
    return 0;
#endif
}

void gpio_set(int gpio)
{
#ifdef __arm__
    if (gpio < 32) {
        GPSET0 = 1 << gpio;
    } else {
        GPSET1 = 1 << (gpio - 32);
    }
#endif
}

void gpio_clear(int gpio)
{
#ifdef __arm__
    if (gpio < 32) {
        GPCLR0 = 1 << gpio;
    } else {
        GPCLR1 = 1 << (gpio - 32);
    }
#endif
}

void gpio_config(int gpio, int config)
{
#ifdef __arm__
    if (gpio < 10) {
        int shift = gpio * 3;
        GPFSEL0 = (GPFSEL0 & ~(7 << shift)) | (config << shift);
    } else if (gpio < 20) {
        int shift = (gpio - 10) * 3;
        GPFSEL1 = (GPFSEL1 & ~(7 << shift)) | (config << shift);
    } else if (gpio < 30) {
        int shift = (gpio - 20) * 3;
        GPFSEL2 = (GPFSEL2 & ~(7 << shift)) | (config << shift);
    }
#endif
}

void gpio_pullups(int* gpios, int count)
{
#ifdef __arm__
    unsigned int clocks0 = 0;
    unsigned int clocks1 = 0;
    int i;
    for (i = 0; i < count; i++) {
        int gpio = gpios[i];
        if (gpio < 32) {
            clocks0 |= (1 << gpio);
        } else {
            clocks1 |= (1 << (gpio - 32));
        }
    }

    GPPUD = 2;
    delay_short();
    GPPUDCLK0 = clocks0;
    GPPUDCLK1 = clocks1;
    delay_short();
    GPPUD = 0;
    GPPUDCLK0 = 0;
    GPPUDCLK1 = 0;
#endif
}

void gpio_pullup(int gpio)
{
    int gpios[1] = {gpio};
    gpio_pullups(gpios, 1);
}

#ifdef LINUX
volatile void *map_memory(unsigned int base, unsigned int size)
{
    int fd;
    volatile void *map;

    if ((fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
        printf("can't open /dev/mem \n");
        exit(-1);
    }

    map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, base);

    close(fd);

    if (map == MAP_FAILED) {
        printf("mmap error\n");
        exit(-1);
    }

    return map;
}
#endif

void gpio_init()
{
#if defined(LINUX) && defined(__arm__)
    peripheral_base = map_memory(BUS_PERIPHERAL_BASE, PERIPHERAL_RANGE);
#endif
}
