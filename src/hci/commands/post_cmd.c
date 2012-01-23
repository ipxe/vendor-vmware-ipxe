/* **********************************************************
 * Copyright 2010 VMware, Inc.  All rights reserved.
 * **********************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <config/general.h>
#include <libgen.h>
#include <getopt.h>
#include <gpxe/refcnt.h>
#include <gpxe/iobuf.h>
#include <gpxe/xfer.h>
#include <gpxe/uri.h>
#include <gpxe/open.h>
#include <gpxe/downloader.h>
#include <gpxe/http.h>
#include <gpxe/monojob.h>
#include <gpxe/image.h>
#include <gpxe/command.h>
#include <gpxe/settings.h>
#include <gpxe/tls.h>
#include <gpxe/dhcp_arch.h>
#include <usr/imgmgmt.h>
#include <gpxe/device.h>
#include <gpxe/pci.h>
#include <gpxe/build.h>
#include <gpxe/base64.h>
#include <gpxe/memmap.h>
#include <gpxe/timer.h>
#include <gpxe/smbios.h>

#include <cpu.h>

/** Boundary for the POST data. */
#define BOUNDARY "---ThisIsTheBoundary--NoneShallPass"

/** Headers for the POST data. */
static const char *POST_HEADER =
	"--" BOUNDARY "\r\n"
	"Content-Disposition: form-data; name=\"config\"\r\n"
	"Content-Type: application/octet-stream\r\n"
	"\r\n";
static const char *POST_FOOTER = "--" BOUNDARY "--\r\n";

/** Stores the body of the POST request. */
struct post_buffer {
	char *data;
	size_t woffset;
	size_t size;
	int errno;
};

/**
 * Append data to the buffer used to store the data in the POST request.
 *
 * @param pb The post_buffer to append the given string to.
 * @param str The string to append to the buffer.
 */
static void post_append(struct post_buffer *pb, const char *str)
{
	int len = strlen ( str );

	if ( ( pb->size < ( pb->woffset + len ) ) ) {
		size_t new_size = pb->woffset + len + 2048;
		char *new_data;

		if ( ( new_data = realloc ( pb->data, new_size ) ) == NULL ) {
			pb->errno = ENOMEM;
			return;
		}
		pb->data = new_data;
		pb->size = new_size;
	}

	memcpy ( &pb->data[pb->woffset], str, len );
	pb->woffset += len;
}

static int read_oem_strings ( struct post_buffer *pb )
{
	struct smbios_structure_node *structure_node = NULL;
	struct smbios_oem_strings oem;
	unsigned int i;
	int rc = 0;

	memset ( &oem, 0, sizeof ( oem ) );
	
	rc = find_smbios_structures ( SMBIOS_TYPE_OEM_INFORMATION,
                                 &structure_node );
	/* If the structure is not found, we don't care. */
	if ( rc != 0 )
		goto cleanup;

	struct smbios_structure_node *tmp = NULL;
	for ( tmp = structure_node; tmp != NULL; tmp = tmp->next ) {
		rc = read_smbios_structure ( &(tmp->structure),
					   &oem, sizeof ( oem ) );
		if ( rc != 0 )
			goto cleanup;

		/* NB: SMBIOS string indexes start at one, zero means "no string". */
		for ( i = 1; i <= oem.count; i++ ) {
			char *buf = NULL;

			/* Get the size of the string. */
			rc = read_smbios_string ( &(tmp->structure),
						  i,
						  NULL,
						  0 );
			if ( rc < 0 )
				goto cleanup;

			if ( ( buf = calloc ( 1, (rc + 1) ) ) == NULL ) {
				rc = -ENOMEM;
				goto cleanup;
			}

			rc = read_smbios_string ( &(tmp->structure),
						  i,
						  buf,
						  rc );
			if ( rc < 0 )
				goto cleanup;

			buf[rc] = '\0';

			post_append ( pb, "oemstring=" );
			post_append ( pb, buf );
			post_append ( pb, "\n" );

			free ( buf );
			rc = 0;
		}
	}

cleanup:
	free_smbios_structures ( &structure_node );
	return rc;
}

static void post_syntax ( char **argv ) {
	printf ( "Usage:\n"
		 "  %s <uri>\n"
		 "\n"
		 "Post machine information and download configuration.\n",
		 argv[0] );
}

static int post_exec ( int argc, char **argv ) {
	struct http_method meth = {
		.method = "POST",
    		.content_type = "multipart/form-data; boundary=" BOUNDARY,
	};

	static char *censored_settings[] = {
		"password",
		"username",
		"reverse-username",
		"reverse-password",
		NULL
	};

	static struct option longopts[] = {
		{ "help", 0, NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};
	struct post_buffer conf_buffer;
	struct setting *setting;
	struct image *image;
	struct uri *uri = NULL;
	char *filename;
	char *name;
	int c;
	int rc = 0;

	/* Parse options */
	while ( ( c = getopt_long ( argc, argv, "h", longopts, NULL ) ) >= 0 ) {
		switch ( c ) {
		case 'h':
			/* Display help text */
		default:
			/* Unrecognised/invalid option */
			post_syntax ( argv );
			return 1;
		}
	}

	/* Need exactly one image name remaining after the options */
	if ( optind != ( argc - 1 ) ) {
		post_syntax ( argv );
		return 1;
	}

	filename = argv[optind++];
	name = basename ( filename );
	
	/* Allocate image */
	image = alloc_image ();
	if ( ! image ) {
		printf ( "%s\n", strerror ( -ENOMEM ) );
		return -ENOMEM;
	}

	/* Fill in image name */
	if ( name ) {
		if ( ( rc = image_set_name ( image, name ) ) != 0 )
			return rc;
	}

	image->type = NULL;

	if ( ! ( uri = parse_uri ( filename ) ) ) {
		rc = -ENOMEM;
		goto error;
	}

	/* Ensure HTTP is being used, we don't support other transports. */
	if ( strcmp ( uri->scheme, "http" ) != 0 &&
	     strcmp ( uri->scheme, "https" ) != 0 &&
	     ( cwuri != NULL &&
	       strcmp ( cwuri->scheme, "http" ) != 0 &&
	       strcmp ( cwuri->scheme, "https" ) != 0 ) ) {
		printf ( "URI is not http.\n" );
		return -ENOTSUP;
	}
	
	image_set_uri ( image, uri );

	/* Start creating the body of the POST request. */
	memset ( &conf_buffer, 0, sizeof(conf_buffer) );
	post_append ( &conf_buffer, POST_HEADER );

	/* Add all the settings. */
	for_each_table_entry ( setting, SETTINGS ) {
		char value[256];
		int j;

		for ( j = 0; censored_settings[j]; j++ ) {
			if ( strcmp ( setting->name,
				      censored_settings[j] ) == 0 )
				break;
		}
		if ( censored_settings[j] )
			continue;
		
		post_append ( &conf_buffer, setting->name );
		post_append ( &conf_buffer, "=" );
		value[0] = '\0';
		fetchf_setting ( NULL,
				 setting,
				 value,
				 sizeof(value) );
		post_append ( &conf_buffer, value );
		post_append ( &conf_buffer, "\n" );
	}

	/* Add gPXE build information. */
	{
		post_append ( &conf_buffer, "build=" );
		post_append ( &conf_buffer, build_string );
		post_append ( &conf_buffer, "\n" );
	}

#ifdef GATHER_RANDOM
	/*
	 * Add random data for verification purposes.
	 */
	{
		char encoded_value[ base64_encoded_len ( sizeof ( random_data_recording ) ) + 1 ];

		base64_encode ( random_data_recording, encoded_value );
		
		post_append ( &conf_buffer, "random=" );
		post_append ( &conf_buffer, encoded_value );
		post_append ( &conf_buffer, "\n" );
	}
#endif

	/* Add CPU information. */
	{
		struct cpuinfo_x86 cpu;
		char cpu_str[128];

		get_cpuinfo ( &cpu );

		snprintf ( cpu_str, sizeof ( cpu_str ),
			   "cpu-vendor=%s\n",
			   cpu.vendor );
		post_append ( &conf_buffer, cpu_str );
		snprintf ( cpu_str, sizeof ( cpu_str ),
			   "cpuid-00000001="
			   "eax:%#08x;"
			   "ebx:%#08x;"
			   "ecx:%#08x;"
			   "edx:%#08x\n",
			   cpu.features[0],
			   cpu.features[1],
			   cpu.features[2],
			   cpu.features[3] );
		post_append ( &conf_buffer, cpu_str );
		snprintf ( cpu_str, sizeof ( cpu_str ),
			   "cpuid-80000001="
			   "ecx:%#08x;"
			   "edx:%#08x\n",
			   cpu.amd_features[0],
			   cpu.amd_features[1] );
		post_append ( &conf_buffer, cpu_str );
	}

	if ( ( rc = read_oem_strings ( &conf_buffer ) != 0 ) ) {
		DBG ( "unable to read OEM strings %x\n", rc );
	}

	/* Add system information. */
	{
		struct memory_map memmap;
		unsigned int i;
		char value[64];

		get_memmap ( &memmap );
		for ( i = 0; i < memmap.count; i++ ) {
			snprintf ( value, sizeof ( value ),
				   "mem-region=%#016llx-%#016llx\n",
				   memmap.regions[i].start,
				   memmap.regions[i].end );
			post_append ( &conf_buffer, value );
		}

		snprintf ( value, sizeof ( value ),
			   "ticks=%ld/%ld\n",
			   currticks (),
			   ticks_per_sec () );
		post_append ( &conf_buffer, value );
	}
	
	/* Add PCI information. */
	{
		struct device *rootdev = 0;
		struct device *dev = 0;

		rootdev = find_root_device ( "PCI" );

		list_for_each_entry ( dev, &rootdev->children, siblings ) {
			struct pci_device *pd = (struct pci_device *)dev;
			char pciid[64];

			snprintf ( pciid, sizeof(pciid),
				   "pciid=%04X:%04X\n",
				   pd->vendor,
				   pd->device );
			post_append ( &conf_buffer, pciid );
		}
	}
	
        /* Add client architecture information. */
        post_append ( &conf_buffer, "arch=" DHCP_ARCH_CLIENT_STRING "\n" );

	post_append ( &conf_buffer, "\r\n" );
	post_append ( &conf_buffer, POST_FOOTER );

	meth.body = conf_buffer.data;
	meth.body_len = conf_buffer.woffset;
	
	if ( ( rc = create_downloader ( &monojob, image,
					register_and_autoexec_image,
					LOCATION_HTTP,
					uri,
					&meth ) ) == 0 )
		rc = monojob_wait ( filename );

 error:
	if ( uri )
	    uri_put ( uri );
	
	if ( rc ) {
		printf ( "Could not fetch %s: %s\n",
			 filename, strerror ( rc ) );
	}
	image_put ( image );

	return 0;
}

struct command post_command __command = {
	.name = "post",
	.exec = post_exec,
};
