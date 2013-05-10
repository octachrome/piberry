COPTS = -marm -mcpu=arm1176jzf-s -mfpu=vfp -Wall -Werror -O2 -nostartfiles -ffreestanding
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
