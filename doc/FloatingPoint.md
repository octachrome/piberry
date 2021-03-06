Floating point
==============

You can't get far doing audio without floating point arithmetic. It's turned off by default, so one of the first things that happens in `start.s` is enabling it. This document goes into quite a bit of detail on how it works, but you don't really need to understand everything to use floating point - just copy the code and skip the rest.

Floating point is implemented using the confusing idea of ARM coprocessors. These are extensions to the ARM core which add support for additional instructions, etc. When the ARM core reads a instruction it cannot understand (like a floating point instruction), it asks the coprocessors if they can execute it, and hopefully they do so. While in some ways floating point on the ARM11 is quite simple, the idea of coprocessors is quite abstract, which makes some of the code used to interact with them hard to understand.

Coprocessors are numbered from 0 to 15. The floating point coprocessor for the ARM11 is called the VFP11, and it appears as two coprocessors, CP10 and CP11. As it says in the manual:

> In general, CP10 is used to encode single-precision operations, and CP11 is used to encode double-precision operations.
> -- ARM ARM, section C1.1

To make things more complicated, the ARM also has a coprocessor called the System Control Coprocessor, CP15, which is often used when interacting with the VFP11 and other peripherals like the memory management unit.

Enabling floating point
-----------------------

To enable floating point, the first thing we do is enable the two coprocessors, CP10 and CP11. We do this by writing to a register in CP15. CP15 has several registers, which are numbered. Register 1 is described in section B3.4, and it is in fact three registers masquerading as one. One of these subregisters is called the Coprocessor Access Register (section B3.4.3), which it is just a bunch of bits which are set to 1 if the corresponding coprocessor is enabled. Now, let me introduce the somewhat confusing `MRC` instruction, which reads the value of a coprocessor register, amongst other things:

        @; Load the contents of the coprocessor access register into r0
        mrc     p15, 0, r0, c1, c0, 2

Aren't there are a lot of operands? Breaking it down:

- `p15` selects CP15 as the coprocessor we want to transfer data from, the system control coprocessor.
- `0` is `opcode1`. As the manual says, "these bits are generally 0 in valid CP15 instructions."
- `r0` is the ARM register we want to transfer data to.
- `c1` is the coprocessor register we want to read from. Register 1 is the cerberus "control registers".
- `c0` is "an additional coprocessor register name which is used for accesses to some primary registers to specify additional information about the version of the register and/or the type of access." Unused here, I think.
- `2` is `opcode2` (or sometimes `opcode_2`). Here it means that we want register 2, the coprocessor access register.

Now we need to enable CP10 and CP11, and then write the value back using `MCR`, the inverse of `MRC`. Each coprocessor has two bits which need setting to 1, so we have an `0b1111` pattern in bits 20-23:

        orr     r0, #0xf00000
        mcr     p15, 0, r0, c1, c0, 2

Coprocessor(s) enabled.

Now we have to enable floating point. Wait, didn't we already do that? The last thing to do is to set the `EN` bit in the `FPEXC` register. This is one of the "normal" floating point registers, which includes 32 general purpose floating point registers and some control/status registers. To modify `FPEXC`, we will use our first floating point instruction, `FMXR`:

        @; enable vfp by setting EN (bit 30)
        mov     r0, #0x40000000
        fmxr    fpexc, r0

Now we can use floating point instructions without raising undefined instruction exceptions. Well, almost.

Floating point support code
---------------------------
> A complete implementation of the VFP architecture must include a software component, known as the
> support code, due to the existence of trapped floating-point exceptions. -- ARM ARM, section C1.2.4

The default floating point mode on the ARM11 is to implement the most common floating point operations in hardware, and delgate to software for special cases. This is done by raising an unsupported operation exception, called a *trap*, in which you the programmer are supposed to figure out what went wrong (e.g., an underflow), calculate the correct result, and resume the program.

If, like me, you don't feel like implementing a bunch of floating point operations, there is an alternative: RunFast mode, or Flush-to-zero mode (which *nearly* means the same thing). This is a pure hardware floating point implementation which is not-quite IEEE 754-compliant. I'm not exactly sure what this means in detail, but I do know that for audio purposes I don't hugely care about the edge cases. Approximate results are OK by me.

Here are two contradicting statements about Flush-to-zero mode:

> Underflow exceptions only occur in Flush-to-zero mode when a result is flushed to zero. They are
> always treated as untrapped, and the Underflow trap enable (UFE) bit in the FPSCR is ignored. -- ARM ARM, section C2.4

> If the FZ bit is set, all underflowing results are forced to a positive signed zero and written to the
> destination register. The UFC flag is set in the FPSCR. No trap is taken. If the Underflow
> exception enable bit is set, it is ignored. -- ARM1176JZF-S Technical Reference Manual, section 22.9.

I believe the second one, since it is more specific to the core used in the BCM2835.

So that you can forget about all this and move on, you need to do two things: disable all the floating point traps, and enable Flush-to-zero. The bits for these are all in `FPSCR`, which is the main floating point status and control register:

        @; load the status register
        fmrx    r0, fpscr
        @; enable flush-to-zero (bit 24)
        orr     r0, #0x01000000
        @; disable traps (bits 8-12 and bit 15)
        bic     r0, #0x9f00
        @; save the status register
        fmxr    fpscr, r0

OK, *now* you can use floating point instructions.

ABIs and compiler options
-------------------------

I compile using the `arm-linux-gnueabihf` version of GCC. The other commonly available ARM GCC target is `arm-linux-gnueabi`. The difference is that they target two different Application Binary Interfaces (ABIs). An ABI is a set of standards about how to call subroutines, how to invoke OS calls, etc. An ABI allows two programs or libraries built with different compilers to reliably work together. In a bare metal environment, you can choose any ABI because you are not linking or running against anyone else's code.

`gnueabihf` is the newest of the ARM GCC targets, and it generates faster calls to subroutines with floating point arguments. This is because the older `gnueabi` allows code which can run on ARM cores with no hardware floating point (these cores use software emulation of floating point instructions), and part of the strategy is to always pass floating point function arguments in the ARM integer registers (because the floating point registers may not exist on some platforms).

Whatever your ABI, compile and assemble with `-mfpu=vfp`, which tells the compiler to generate VFPv2 instructions. Without this flag, I have seen the compiler generate VFPv3 instructions, which are not implemented on the BCM2835.

See also
--------

- [ARM Architecture Reference Manual](https://www.scss.tcd.ie/~waldroj/3d1/arm_arm.pdf), section C (Google "ARM DDI 0100i" if the link is broken).
- https://wiki.debian.org/ArmHardFloatPort and https://wiki.debian.org/ArmEabiPort, for more info on ARM ABIs.
