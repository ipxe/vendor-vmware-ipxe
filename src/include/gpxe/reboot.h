#ifndef _GPXE_REBOOT_H
#define _GPXE_REBOOT_H

/** @file
 *
 * Rebooting the machine
 *
 */

FILE_LICENCE ( GPL2_OR_LATER );

#include <gpxe/api.h>
#include <config/general.h>

/**
 * Provide a rebot API implementation
 *
 * @v _prefix		Subsystem prefix
 * @v _api_func		API function
 * @v _func		Implementing function
 */
#define PROVIDE_REBOOT( _subsys, _api_func, _func ) \
	PROVIDE_SINGLE_API ( REBOOT_PREFIX_ ## _subsys, _api_func, _func )

/* Include all architecture-independent reboot API headers */
#include <gpxe/efi/efi_reboot.h>

/* Include all architecture-dependent reboot API headers */
#include <bits/reboot.h>

extern void reboot ( void );

#endif /* _GPXE_REBOOT_H */
