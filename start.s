.text

.global bare_start

bare_start:
	ldr		sp, =stack

	@; enable single & double precision vfp
	mrc		p15 , 0, r0, c1, c0, 2
	orr		r0, #0xf00000
	mcr		p15, 0, r0, c1, c0, 2
	@; enable vfp
	mov 	r0, #0x40000000
	fmxr	fpexc, r0

	bl		notmain
loop:
	b		loop

.global raise

raise:
	b		raise

.global dummy

dummy:
	bx		lr

.data

.global stack

.space 0x1000,0xbb
stack:
.space 4,0xbb
