#ifndef _GPXE_EFI_REBOOT_H
#define _GPXE_EFI_REBOOT_H

/** @file
 *
 * EFI reboot
 *
 */

FILE_LICENCE ( GPL2_OR_LATER );

#ifdef REBOOT_EFI
#define REBOOT_PREFIX_efi
#else
#define REBOOT_PREFIX_efi __efi_
#endif

#endif /* _GPXE_EFI_REBOOT_H */
