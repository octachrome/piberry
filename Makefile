# -mfpu=vfp is required, otherwise the compiler will generate illegal instructions, such as vmov.f32
COPTS = -marm -mcpu=arm1176jzf-s -mfpu=vfp -Wall -Werror -O2 -nostartfiles -ffreestanding
# -mfpu=vfp is required to make the assembler accept the fpexc instruction as legal
AOPTS = -mcpu=arm1176jzf-s -mfpu=vfp
OBJS = start.o wave.o sampletable.o

all : wave.bin

clean :
	-rm *.o *.elf *.bin

%.o : %.s
	$(ARMGNU)-as $(AOPTS) -o $@ $<

%.o : %.c
	$(ARMGNU)-gcc $(COPTS) -c $<

wave.elf : $(OBJS) rpi.ld
	$(ARMGNU)-ld -o wave.elf $(OBJS) -T rpi.ld -static -L/home/chris/code/pi-baremetal/rpi-libgcc -lgcc

%.bin : %.elf
	$(ARMGNU)-objcopy -O binary $< $@
