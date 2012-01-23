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

#include <gpxe/efi/efi.h>
#include <gpxe/reboot.h>

FILE_LICENCE ( GPL2_OR_LATER );

static void efi_reboot ( void ) {
	efi_systab->RuntimeServices->ResetSystem ( EfiResetCold, EFI_SUCCESS,
                                                   0, NULL );
}

PROVIDE_REBOOT ( efi, reboot, efi_reboot );
