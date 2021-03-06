# -mfpu=vfp is required, otherwise the compiler will generate illegal instructions, such as vmov.f32
COPTS = -g -marm -mcpu=arm1176jzf-s -mfpu=vfp -Wall -Werror -fsingle-precision-constant $(PI_DEFS)
#  -nostartfiles -ffreestanding 
# -mfpu=vfp is required to make the assembler accept the fpexc instruction as legal
AOPTS = -g -mcpu=arm1176jzf-s -mfpu=vfp
OBJS = start.o test.o pwm.o module.o sine.o envelope.o multiply.o gpio.o kbd.o simple.o noise.o

all : clean kernel.img

clean :
	-rm *.o *.elf *.img

%.o : %.c
	$(ARMGNU)-gcc $(COPTS) -c $<

%.o : patches/%.c
	$(ARMGNU)-gcc $(COPTS) -c $<

%.o : %.s
	$(ARMGNU)-as $(AOPTS) -o $@ $<

kernel.elf : $(OBJS) rpi.ld
	$(ARMGNU)-ld -o kernel.elf $(OBJS) -T rpi.ld -static -L/home/chris/code/pi-baremetal/rpi-libgcc -lgcc

%.img : %.elf
	$(ARMGNU)-objcopy -O binary $< $@

install : kernel.img
	cp kernel.img /media/chris/3866-C336/kernel.img
	sync
	-eject /media/chris/3866-C336

alsa_test : alsa.c test.c module.c sine.c envelope.c multiply.c kbd.c gpio.c noise.c patches/simple.c patches/kick.c patches/fm.c patches/snare.c
	cc -DLINUX -o $@ -g $^ -lasound

run : alsa_test
	sudo ./alsa_test
