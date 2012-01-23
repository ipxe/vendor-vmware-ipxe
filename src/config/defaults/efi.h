#ifndef CONFIG_DEFAULTS_EFI_H
#define CONFIG_DEFAULTS_EFI_H

/** @file
 *
 * Configuration defaults for EFI
 *
 */

#define UACCESS_EFI
#define IOAPI_X86
#define PCIAPI_EFI
#define TIMER_EFI
#define NAP_EFIX86
#define UMALLOC_EFI
#define SMBIOS_EFI
#define REBOOT_EFI

#define CONSOLE_EFI
#define CONSOLE_SERIAL

#define	IMAGE_EFI		/* EFI image support */
#define IMAGE_SCRIPT		/* gPXE script image support */

#endif /* CONFIG_DEFAULTS_EFI_H */
