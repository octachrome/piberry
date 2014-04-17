.text

.global bare_start

@; The next 16 words will be copied to address 0. The ldr instruction will use relative addressing
@; to indicate the adress being loaded from, so we preserve the offset between the instructions and
@; the corresponding vectors by copying both. The vector variables themselves then contain absolute
@; addresses of the handler routines.

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
	.word	bare_start

undef_vector:
	.word	undef_instr_handler

swi_vector:
	.word	loop

prefetch_abort_vector:
	.word	loop

data_abort_vector:
	.word	loop

nothing:
	.word	0

irq_vector:
	.word	irq_handler_blink

fiq_vector:
	.word	loop

bare_start:
	@; enable single & double precision vfp using coprocessor access control register
	mrc		p15, 0, r0, c1, c0, 2
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

	@; install interrupt handlers
	mov		r0, #0x8000
	mov		r1, #0
	ldm		r0!, {r2, r3, r4, r5, r6, r7, r8, r9}
	stm		r1!, {r2, r3, r4, r5, r6, r7, r8, r9}
	ldm		r0!, {r2, r3, r4, r5, r6, r7, r8, r9}
	stm		r1!, {r2, r3, r4, r5, r6, r7, r8, r9}

	bl		notmain
loop:
	b		loop

irq_handler_blink:
	push	{r0, r1, r2, lr}
	bl		blink_fast
	mov		r0, #0xffffffff
	ldr		r1, =0x20200040		@; GPEDS0
	str		r0, [r1]
	pop		{r0, r1, r2, lr}
	subs	pc, lr, #4

blink_fast:
	mov		r2, #3
	@; Set GPIO16 for output using GPFSEL1
	ldr		r1, =0x20200004
	ldr		r0, [r1]
	bic		r0, r0, #0x1c0000
	orr		r0, r0, #0x040000
	str		r0, [r1]

blink2:
	@; Set GPIO16 using GPSET0
	ldr		r1, =0x2020001C
	mov		r0, #0x10000
	str		r0, [r1]

	mov		r0, #0x100000
blink3:
	subs	r0, r0, #1
	bne		blink3

	@; Clear GPIO16 using GPCLR0
	ldr		r1, =0x20200028
	mov		r0, #0x10000
	str		r0, [r1]

	mov		r0, #0x100000
blink4:
	subs	r0, r0, #1
	bne		blink4

	subs	r2, r2, #1
	bne		blink2
	bx		lr


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

.space 0x1000,0xbb
user_stack:
.space 4,0xbb

.space 0x1000,0xbb
irq_stack:
.space 4,0xbb

.space 0x1000,0xbb
und_stack:
.space 4,0xbb
