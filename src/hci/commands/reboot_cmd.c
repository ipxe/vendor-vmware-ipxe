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

#include <stdio.h>
#include <getopt.h>
#include <gpxe/command.h>
#include <gpxe/reboot.h>

/** @file
 *
 * Reboot command
 *
 */

/**
 * "reboot" command syntax message
 *
 * @v argv		Argument list
 */
static void reboot_syntax ( char **argv ) {
	printf ( "Usage:\n"
		 "  %s\n"
		 "Reboot the machine.\n",
		 argv[0] );
}

/**
 * The "reboot" command
 *
 * @v argc		Argument count
 * @v argv		Argument list
 * @ret rc		Exit code
 */
static int reboot_exec ( int argc, char **argv ) {
	static struct option longopts[] = {
		{ "help", 0, NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};
	int c;

	/* Parse options */
	while ( ( c = getopt_long ( argc, argv, "h", longopts, NULL ) ) >= 0 ){
		switch ( c ) {
		case 'h':
			/* Display help text */
		default:
			/* Unrecognised/invalid option */
			reboot_syntax ( argv );
			return 1;
		}
	}

	reboot ();
	return 0;
}

/** GDB stub commands */
struct command reboot_command __command = {
	.name = "reboot",
	.exec = reboot_exec,
};
