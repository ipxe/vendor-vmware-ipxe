/*
 * Copyright (C) 2010 VMware, Inc.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <gpxe/reboot.h>
#include <realmode.h>

FILE_LICENCE ( GPL2_OR_LATER );

static void bios_reboot ( void ) {
	uint16_t *const reboot_flag = (uint16_t *) 0x472;

	*reboot_flag = 0; // cold reboot
	__asm__ __volatile__ ( REAL_CODE ( "pushw $0xf000\n\t"
					   "pushw $0xfff0\n\t"
					   "lretw\n\t" ) : : );
}

PROVIDE_REBOOT ( pcbios, reboot, bios_reboot );
