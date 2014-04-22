#include "audio.h"
#include "pi.h"

#define PULSES_PER_SAMPLE   2048

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

    // enable dma, and raise dreq as soon as only 7 words left in the fifo
    PWM_DMAC = 0x80000707;
}

typedef struct {
    unsigned int transfer_info;
    unsigned int source_ad;
    unsigned int dest_ad;
    unsigned int txfr_len;
    unsigned int stride;
    unsigned int nextconblk;
    unsigned int reserved1;
    unsigned int reserved2;
} dma_cb_t;

// Defined in start.s so I can guarantee 8-byte alignment
extern dma_cb_t dma_control_blocks[];
unsigned int pwm_buffer0[BLOCK_SIZE];
unsigned int pwm_buffer1[BLOCK_SIZE];
unsigned int dma_cb0_addr;
unsigned int dma_cb1_addr;
unsigned int dma_last_filled;

static void configure_dma()
{
    dma_cb0_addr = ((unsigned int) &dma_control_blocks[0]) + RAM_PHYS_OFFSET;
    dma_cb1_addr = ((unsigned int) &dma_control_blocks[1]) + RAM_PHYS_OFFSET;

    // 5 = pwm channel, 1 = src_inc, 4 = dest_dreq, 8 = wait_resp
    dma_control_blocks[0].transfer_info = 0x00050148;
    dma_control_blocks[0].source_ad = ((unsigned int) pwm_buffer0) + RAM_PHYS_OFFSET;
    dma_control_blocks[0].dest_ad = 0x7E20C018; // PWM_FIFO physical address
    dma_control_blocks[0].txfr_len = BLOCK_SIZE * sizeof(int);
    dma_control_blocks[0].stride = 0;
    dma_control_blocks[0].nextconblk = dma_cb1_addr;
    dma_control_blocks[0].reserved1 = 0;
    dma_control_blocks[0].reserved2 = 0;

    dma_control_blocks[1].transfer_info = 0x00050148;
    dma_control_blocks[1].source_ad = ((unsigned int) pwm_buffer1) + RAM_PHYS_OFFSET;
    dma_control_blocks[1].dest_ad = 0x7E20C018;
    dma_control_blocks[1].txfr_len = BLOCK_SIZE * sizeof(int);
    dma_control_blocks[1].stride = 0;
    dma_control_blocks[1].nextconblk = dma_cb0_addr;
    dma_control_blocks[1].reserved1 = 0;
    dma_control_blocks[1].reserved2 = 0;

    dma_last_filled = 0;

    // reset dma controller
    DMA0_CS = 0x80000000;
}

void audio_init()
{
    configure_pwm_clock();
    configure_pwm();
    configure_dma();
}

void audio_write(float* block)
{
    // Wait for the buffers to flip
    while (DMA0_CONBLK_AD != dma_last_filled);

    unsigned int* pwm_buffer;
    if (DMA0_CONBLK_AD == dma_cb0_addr) {
        pwm_buffer = pwm_buffer1;
        dma_last_filled  = dma_cb1_addr;
    } else {
        pwm_buffer = pwm_buffer0;
        dma_last_filled = dma_cb0_addr;
    }

    int i;
    for (i = 0; i < BLOCK_SIZE; i++) {
        pwm_buffer[i] = PULSES_PER_SAMPLE * ((1 + block[i]) / 2);
    }

    // Start DMA if not already active
    if (!(DMA0_CS & 1)) {
        // clear end flag
        DMA0_CS |= 2;

        // set control block
        DMA0_CONBLK_AD = dma_last_filled;

        // enable dma
        DMA0_CS |= 1;
    }
}

// Write PWM data via the FIFO register. Slow and no longer used.
void audio_write_fifo(float* block)
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
