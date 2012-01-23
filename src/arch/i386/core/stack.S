FILE_LICENCE ( GPL2_OR_LATER )

	.arch i386

/****************************************************************************
 * Internal stack
 ****************************************************************************
 */
	.section ".stack", "aw", @nobits
	.align 8
	.globl _stack
_stack:
	.space 8192
	.globl _estack
_estack:
