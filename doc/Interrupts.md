Interrupts
==========

The MPR121 touch sensor has an IRQ output pin. I wanted to connect this to a GPIO pin and then have it trigger some interrupt handler code whenever the pin level changes. This took me many hours to achieve, and this is what I learned.

Like many CPUs, ARM interrupt vectors are stored in a block of memory that starts at address 0. Unlike other CPUs, ARM interrupt vectors are not addresses of interrupt handlers, they are branching instructions. This means you have some options about what you put there.

The obvious choice is a branch:

        b   my_handler

Installing an interrupt handler is a bit awkward, because you have to calculate the opcode for the branch instruction, which contains the relative address of the handler, measured from the interrupt vector location. I got the magic formula from http://wiki.osdev.org/ARM_Integrator-CP_IRQTimerAndPIC:

        0xea000000 | ((((unsigned int) handler_address) - (8 + 4 * index)) >> 2)

Make sure you cast `handler_address` to an int so that you are doing byte arithmetic, not pointer arithmetic.

This method is limited to jumps within the first 32Mb of RAM due to the size of relative offsets that can be encoded in the instruction.

I use an alternative method from [David Welch](https://github.com/dwelch67/raspberrypi/blob/master/blinker07/vectors.s), which is to store the addresses of your interrupt handlers in a table, and then use `ldr` to load them into the program counter. The assembler looks like this:

        interrupt_vectors:
            ldr pc, bare_start_addr
            ldr pc, undef_vector
            ldr pc, swi_vector
            ldr pc, prefetch_abort_vector
            ldr pc, data_abort_vector
            ldr pc, nothing
            ldr pc, irq_vector
            ldr pc, fiq_vector

        bare_start_addr:
            .word   bare_start

        undef_vector:
            .word   0

        swi_vector:
            .word   0

        prefetch_abort_vector:
            .word   0

        data_abort_vector:
            .word   0

        nothing:
            .word   0

        irq_vector:
            .word   0

        fiq_vector:
            .word   0

        ...

        bare_start:
            @; this is the start of the program

Install an interrupt handler by simply writing the handler address to the appropriate table entry. E.g.:

        void* irq_vector = 0x38;
        ...
        irq_vector = (void*) my_handler;

To use this method you have to get the jump table installed at the start of RAM. There are two options here:

#### Option 1: load your program at address 0

Put the jump table listed above at the very start of your program. Then change your linker script so that it links the code section to start at address 0:

        MEMORY
        {
            ram : ORIGIN = 0x0, LENGTH = ...

Then, edit `config.txt` on your SD card and add/uncomment the following line:

        disable_commandline_tags=1

This setting does two things: it causes the bootloader to load your program image at address 0, and it stops the bootloader from trashing your code at address 0x100. The region starting at address 0x100 is where the bootloader normally writes the ATAGs, which are parameters that tell the Linux kernel about the platform it is booting on.

NB: the instruction at address 0 will be executed when the ARM core is first enabled, so make sure it jumps to your initialisation code. The listing above does this by initialising the first table entry to the address of my start routine, `bare_start`.

You might notice in your config.txt another option called `kernel_address`. This can be used to load your program image at other addresses, other than 0 and 0x8000. It only seems to work if you also set `disable_commandline_tags`. Not that this helps you to install interrupt handlers; it's just a point of interest.

#### Option 2: load your program elsewhere, then install the jump table with code

Put the interrupt vector code anywhere in your program. Then, in your startup code, copy the interrupt vectors to address 0. The `ldr pc, xxx` instructions use relative addressing to access the table entries, so when you move the instructions to address 0, the table of interrupt handler addresses will also need to move.

`ldm` is your friend here: it loads consecutive words of memory into some registers, and `stm` writes them back again. Given the assembler above, we need to move 16 words of memory (8 `ldr` instructions, plus 8 table entries):

        @; r0 holds the address we will load from
        ldr     r0, =interrupt_vectors
        @; r1 holds the address we will store to
        mov     r1, #0
        @; load r2-r9 with the first 8 words of memory starting at 0x8000
        ldm     r0!, {r2, r3, r4, r5, r6, r7, r8, r9}
        @; and store them at 0x0
        stm     r1!, {r2, r3, r4, r5, r6, r7, r8, r9}
        @; and load the next 8 words
        ldm     r0!, {r2, r3, r4, r5, r6, r7, r8, r9}
        @; and store
        stm     r1!, {r2, r3, r4, r5, r6, r7, r8, r9}

The `!` symbol after `r0` and `r1` means that these registers should be incremented after the load/store operation. They are incremented by however many bytes were read/written altogether. After the first `ldm`, `r0` will be incremented to interrupt_vectors+0x20, ready to load the next block of memory. After the first `stm`, r1 will be incremented to 0x20, ready to store the next block.

TODO
----

IRQs, GPIO ranges, clearing interrupt flags.

A co-incidence
--------------

While I was experimenting with this, something strange was happening. I could install a mostly-working IRQ handler by writing it at address 0x38 (it crashed occasionally), but without setting up the `ldr`s which make up the interrupt vectors. My program image was being loaded at 0x8000, and I was not writing anything at all to addresses 0-0x20.

I realised that the bootloader populates the area from address 0 with the contents of a file called first32k.bin. It's disassembly starts like this:

           0:   ea000006    b   0x20
           4:   e1a00000    nop         ; (mov r0, r0)
           8:   e1a00000    nop         ; (mov r0, r0)
           c:   e1a00000    nop         ; (mov r0, r0)
          10:   e1a00000    nop         ; (mov r0, r0)
          14:   e1a00000    nop         ; (mov r0, r0)
          18:   e1a00000    nop         ; (mov r0, r0)
          1c:   e1a00000    nop         ; (mov r0, r0)
          20:   e3a00000    mov r0, #0
          24:   e3a01042    mov r1, #66 ; 0x42
          28:   e3811c0c    orr r1, r1, #12, 24 ; 0xc00
          2c:   e59f2000    ldr r2, [pc]    ; 0x34
          30:   e59ff000    ldr pc, [pc]    ; 0x38
          34:   00000100    andeq   r0, r0, r0, lsl #2
          38:   00008000    andeq   r8, r0, r0

The instruction at address 0x30 loads the program counter with 0x8000, to jump to the start of the program image. But it gets address 0x8000 from the memory location at 0x38 - exactly where I was installing my interrupt handler. The interrupt vectors themselves are nops, so the code would just run down and set pc to my interrupt handler; what a co-incidence. The handler crashed sometimes because the code between the nops and the jump trashes r0, r1 and r2, but not as often as I would have expected.
