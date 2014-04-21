# -mfpu=vfp is required, otherwise the compiler will generate illegal instructions, such as vmov.f32
COPTS = -g -marm -mcpu=arm1176jzf-s -mfpu=vfp -Wall -Werror -fsingle-precision-constant $(PI_DEFS)
#  -nostartfiles -ffreestanding 
# -mfpu=vfp is required to make the assembler accept the fpexc instruction as legal
AOPTS = -g -mcpu=arm1176jzf-s -mfpu=vfp
OBJS = start.o test.o pwm.o module.o sine.o envelope.o multiply.o

all : wave.bin

clean :
	-rm *.o *.elf *.bin

%.o : %.c
	$(ARMGNU)-gcc $(COPTS) -c $<

%.o : %.s
	$(ARMGNU)-as $(AOPTS) -o $@ $<

wave.elf : $(OBJS) rpi.ld
	$(ARMGNU)-ld -o wave.elf $(OBJS) -T rpi.ld -static -L/home/chris/code/pi-baremetal/rpi-libgcc -lgcc

wave.linux.elf : $(OBJS)
	$(ARMGNU)-gcc -o wave.linux.elf $(COPTS) $(OBJS)

%.bin : %.elf
	$(ARMGNU)-objcopy -O binary $< $@

install : wave.bin
	cp wave.bin /media/chris/C522-EA52/kernel.img
	sync
	-eject /media/chris/C522-EA52

alsa_test : alsa.c test.c module.c sine.c envelope.c multiply.c
	cc -DLINUX -o $@ -g $^ -lasound
