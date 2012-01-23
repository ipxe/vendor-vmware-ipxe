#include <stdint.h>
#include <string.h>
#include <cpu.h>

/** @file
 *
 * CPU identification
 *
 */

/**
 * Test to see if CPU flag is changeable
 *
 * @v flag		Flag to test
 * @ret can_change	Flag is changeable
 */
static inline int flag_is_changeable ( unsigned long flag ) {
	unsigned long f1, f2;

        /* This code will compile for either a 32 or 64 bit target. The
         * assembler will automatically use the default operand size in each
         * case.
         */
	__asm__ ( "pushf\n\t"
		  "pushf\n\t"
		  "pop %0\n\t"
		  "mov %0,%1\n\t"
		  "xor %2,%0\n\t"
		  "push %0\n\t"
		  "popf\n\t"
		  "pushf\n\t"
		  "pop %0\n\t"
		  "popf\n\t"
		  : "=&r" ( f1 ), "=&r" ( f2 )
		  : "ir" ( flag ) );

	return ( ( ( f1 ^ f2 ) & flag ) != 0 );
}

/**
 * Get CPU information
 *
 * @v cpu		CPU information structure to fill in
 */
void get_cpuinfo ( struct cpuinfo_x86 *cpu ) {
	unsigned int cpuid_level;
	unsigned int cpuid_extlevel;
	unsigned int discard_1, discard_2, discard_3;
	unsigned int vendor[3];

	memset ( cpu, 0, sizeof ( *cpu ) );

	/* Check for CPUID instruction */
	if ( ! flag_is_changeable ( X86_EFLAGS_ID ) ) {
		DBG ( "CPUID not supported\n" );
		return;
	}

	/* Get features, if present */
	cpuid ( 0x00000000, &cpuid_level, &vendor[0], &vendor[2], &vendor[1] );
	memcpy ( &cpu->vendor[0], &vendor[0], sizeof ( vendor[0] ) );
	memcpy ( &cpu->vendor[4], &vendor[1], sizeof ( vendor[1] ) );
	memcpy ( &cpu->vendor[8], &vendor[2], sizeof ( vendor[2] ) );
	cpu->vendor[12] = '\0';
	if ( cpuid_level >= 0x00000001 ) {
		cpuid ( 0x00000001,
			&cpu->features[0], &cpu->features[1],
			&cpu->features[2], &cpu->features[3] );
	} else {
		DBG ( "CPUID cannot return capabilities\n" );
	}

	/* Get 64-bit features, if present */
	cpuid ( 0x80000000, &cpuid_extlevel, &discard_1,
		&discard_2, &discard_3 );
	if ( ( cpuid_extlevel & 0xffff0000 ) == 0x80000000 ) {
		if ( cpuid_extlevel >= 0x80000001 ) {
			cpuid ( 0x80000001, &discard_1, &discard_2,
				&cpu->amd_features[0],
				&cpu->amd_features[1] );
		}
	}
}
