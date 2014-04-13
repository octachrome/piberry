.text

.global bare_start

bare_start:

	@; enable single & double precision vfp
	mrc		p15 , 0, r0, c1, c0, 2
	orr		r0, #0xf00000
	mcr		p15, 0, r0, c1, c0, 2

	@; enable vfp
	mov 	r0, #0x40000000
	fmxr	fpexc, r0

	@; enable flush-to-zero (24)
	@; disable traps for underflow (11), inexact (12), and input denormal (15)
	fmrx	r0, fpscr
	orr		r0, #0x01000000
	bic		r0, #0x9800
	fmxr	fpscr, r0

	@; change to irq mode and set up stack
	cps		#18
	ldr		sp, =irq_stack

	@; change to undefined mode and set up stack
	cps		#27
	ldr		sp, =und_stack

	@; enable irq, change to user mode
	cpsie	i, #16
	ldr		sp, =user_stack
	bl		notmain
loop:
	b		loop

.global undef_instr_handler

@; when a floating-point exception occurs, clear exception flags and resume
undef_instr_handler:
	push	{r0}
	mov		r0, #0x40000000
	fmxr	fpexc, r0
	pop		{r0}
	subs	pc, lr, #4

.global raise

@; Needed by libgcc functions, like ldiv0
raise:
	b		raise

.global dummy

dummy:
	bx		lr

.data

.global user_stack, irq_stack

.space 0x1000,0xbb
user_stack:
.space 4,0xbb

.space 0x1000,0xbb
irq_stack:
.space 4,0xbb

.space 0x1000,0xbb
und_stack:
.space 4,0xbb
