#ifndef _GPXE_BIOS_REBOOT_H
#define _GPXE_BIOS_REBOOT_H

/** @file
 *
 * BIOS reboot
 *
 */

FILE_LICENCE ( GPL2_OR_LATER );

#ifdef REBOOT_PCBIOS
#define REBOOT_PREFIX_pcbios
#else
#define REBOOT_PREFIX_pcbios __pcbios_
#endif

#endif /* _GPXE_BIOS_REBOOT_H */
