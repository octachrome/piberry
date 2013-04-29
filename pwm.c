//  Print out PWM config registers. Hacked by Chris from:
//
//  How to access GPIO registers from C-code on the Raspberry-Pi
//  Example program
//  15-January-2012
//  Dom and Gert
//  Revised: 15-Feb-2013


// Access from ARM Running Linux

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#define PWM_BASE                 (BCM2708_PERI_BASE + 0x20C000) /* PWM controller */
#define CLOCK_BASE               (BCM2708_PERI_BASE + 0x101000) /* GPIO controller */

#define PWM_CONTROL 0
#define PWM_STATUS  1
#define PWM0_RANGE  4
#define PWM0_DATA   5
#define PWM_FIFO    6
#define PWM1_RANGE  8
#define PWM1_DATA   9

#define PWMCLK_CNTL   40
#define PWMCLK_DIV    41

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

// I/O access
volatile unsigned *gpio;
volatile unsigned *clk;
volatile unsigned *pwm;


// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define PRINT(base,offset) printf("%s: %u\n", #offset, *(base+offset))

volatile unsigned *map_memory();

int main(int argc, char **argv)
{
  // Set up gpi pointer for direct register access
  gpio = map_memory(GPIO_BASE);
  clk = map_memory(CLOCK_BASE);
  pwm = map_memory(PWM_BASE);

  // values from forum
  //PWMCLK_CNTL = 148 = 10010100
  //PWMCLK_DIV = 16384
  //PWM_CONTROL=9509 = 10010100100101
  //PWM0_RANGE=1024
  //PWM1_RANGE=1024

  // CB measured values while playing audio (control is 0 when not playing)
  // PWMCLK_CNTL: 148 = 10010100 (SRC=PLLA,ENAB,BUSY,MASH=0) 
  // PWMCLK_DIV: 16384 = 100.000000000000 = 4 (I think)
  // PWM_CONTROL: 8481 = 10000100100001 (enable and use fifo for both channels)
  // PWM0_RANGE: 2048
  // PWM1_RANGE: 2048

  // page 105 of peripherals datasheet implies:
  // PLLA = 650 MHz
  // PLLB = 400 MHz
  // PLLC = 200 MHz
  // PLLD = 500 MHz

  // 2048 gives a resolution of 11 bits
  // 2048 pulses are generated at a rate of 650MHz / 4 / 2048 = 79KHz
  // This is about double the audio sample rate, which is odd.
  // I don't think this has anything to do with stereo - each audio channel uses its own PWM channel.

  PRINT(clk,PWMCLK_CNTL);
  PRINT(clk,PWMCLK_DIV);
  PRINT(pwm,PWM_CONTROL);
  PRINT(pwm,PWM0_RANGE);
  PRINT(pwm,PWM1_RANGE);

  return 0;

} // main


//
// Set up a memory regions to access GPIO, etc.
//
volatile unsigned *map_memory(unsigned int base)
{
  int  fd;
  void *map;

   /* open /dev/mem */
   if ((fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
   }

   /* mmap GPIO */
   map = mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      fd,           //File to map
      base         //Offset to peripheral
   );

   close(fd); //No need to keep mem_fd open after mmap

   if (map == MAP_FAILED) {
      printf("mmap error %d\n", (int)map);//errno also set!
      exit(-1);
   }

   // Always use volatile pointer!
   return (volatile unsigned *)map;
}

