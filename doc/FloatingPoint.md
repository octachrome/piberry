Floating point
==============

You can't get far doing audio without floating point arithmetic. It's turned off by default, so one of the first things that happens in `start.s` is enabling it.

Floating point is implemented using the confusing idea of ARM coprocessors. These are extensions to the ARM core which add support for additional instructions, etc. When the ARM core reads a instruction it cannot understand (like a floating point instruction), it asks the coprocessors if they can execute it, and hopefully they do so. While in some ways floating point on the ARM11 is quite simple, the idea of coprocessors is quite abstract, which makes some of the code used to interact with them hard to understand.

Coprocessors are numbered from 0 to 15. The floating point coprocessor for the ARM11 is called the VFP11, and it appears as two coprocessors, CP10 and CP11. As it says in the manual:

> In general, CP10 is used to encode single-precision operations, and CP11 is used to encode double-precision operations.
> -- ARM ARM section C1.1

To make things more complicated, the ARM also has a coprocessor called the System Control Coprocessor, CP15, which is often used when interacting with the VFP11 and other peripherals like the memory management unit.

Enabling floating point
-----------------------

To enable floating point, the first thing we do is enable the two coprocessors, CP10 and CP11. We do this by writing to a register in CP15. CP15 has several registers, which are numbered. Register 1 is described in section B3.4, and it is in fact three registers masquerading as one. One of these subregisters is called the Coprocessor Access Register (section B3.4.3), which it is just a bunch of bits which are set to 1 if the corresponding coprocessor is enabled. Now, let me introduce the `MRC` instruction, which reads the value of a coprocessor register, amongst other things:

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

Now we have to enable floating point. Wait, didn't we already do that? The last thing to do is to set the `EN` bit in the `FPEXC` register. This is one of the "normal" floating point registers, which includes 32 general purpose floating point registers and some control/status registers. To set `FPEXC`, we will use our first floating point instruction, `FMXR`:

        @; enable vfp
        mov     r0, #0x40000000
        fmxr    fpexc, r0

Now we can use floating point instructions without raising undefined instruction exceptions.
