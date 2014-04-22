MEMORY
{
	ram : ORIGIN = 0x8000, LENGTH = 0x40000
}

SECTIONS
{
	.text : { *(.text*) } > ram
	.bss : { *(.bss*) } > ram
	.data : { *(.data*) } > ram
}

ENTRY(bare_start)
