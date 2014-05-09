This project needs a name.
==========================

Experiments in audio synthesis on a baremetal Raspberry Pi. I started out wanting to create a drum machine that would be intuitive and fun to use. I might do that, or I might build a general purpose synthesizer instead, or something different.

Right now it plays a sythesized kick drum sound through the Pi headphone port at 120bpm.

Current features are:

- 12-bit PWM audio using DMA.
- A basic modular audio pipeline (sine wave VCO, attack-decay envelope).
- Very basic GPIO interrupt handling (if you ground GPIO 17, it plays the kick drum).
- All DIY - no Linux, no third-party libraries (well, apart from libgcc).

Coming soon:

- Interface with MPR121 touch sensor board.
- More audio modules, and more drum sounds.

Build instructions
------------------

### Building for Pi on x86 Ubuntu with a cross-compiler

- Install the ARM cross-compiler

        sudo apt-get install gcc-arm-linux-gnueabihf

- Get a copy of libgcc (GCC needs it for integer division, etc.) from Raspbian, or [Github](https://github.com/brianwiddas/pi-baremetal/tree/master/rpi-libgcc).

- Set `ARMGNU` to the prefix of the compiler you installed:

        export ARMGNU=arm-linux-gnueabihf

- Edit the makefile and correct the path where `libgcc.a` can be found.
- Make the binary image file, kernel.img:

        make

- Format an SD card with FAT16 or FAT32 (I had problems with two FAT12 cards until I reformatted them with FAT16).

- Copy the `kernel.img` you just built onto the SD card, along with the regular bootloader files, `start.elf` and `bootcode.bin` (you can get these from https://github.com/raspberrypi/firmware/tree/master/boot.)

- Plug your SD card, headphones and power cable into your Pi, and listen.

### Using ALSA

For testing, you can also build the audio pipeline on x86 (or whatever) Linux, using ALSA for audio.

- Run `aplay -l` and pick which sound card you want to play audio with. Note the card number and subdevice number.

- Edit `alsa.c` and change `hw1,1` to whatever numbers you chose.

- Build and run:

        make alsa_test
        ./alsa_test


Project structure
-----------------

### start.s
Initial setup. Enable floating point. Install interrupt handlers. Simplest ever `malloc` implementation.

### test.c
Contains the `main` function. Creates a simple kick synth and starts playing it.

### pwm.c
Implements the `audio_` interface (defined in `audio.h`) using PWM. Creates a circular buffer using two DMA control blocks which copy two int arrays to the PWM peripheral.

### alsa.c
Implements the `audio_` interface using ALSA.

### module.c
Implements the high-level modular audio API, `module_`. The basic idea to create audio in blocks of, say, 32 frames (one frame is a left channel sample and a right channel sample). In one iteration of the pipeline, all the modules create a block of audio data, potentially taking their input from blocks created by other modules. The output from some final mixer module gets sent to the `audio_` interface.

When you create a module you get a handle which you can use to read that module's current block. When you ask the module API for a module's current block, it lazily asks the module to populate it, if it hasn't already done so.

There is also a very basic trigger API, for triggering envelopes when a key is pressed or pad hit, etc.

### sine.c, multiply.c, envelope.c
Implementation of some very basic audio modules, from which simple synthesizers can be built.

### docs/*
Notes on what I have learned about baremetal Pi programming so far.

Thanks
------

I learned a lot initially from two great baremetal Pi projects:

- https://github.com/brianwiddas/pi-baremetal
- https://github.com/dwelch67/raspberrypi

So, thanks to Brian and David.
