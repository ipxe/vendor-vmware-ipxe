#ifndef _GPXE_HTTP_H
#define _GPXE_HTTP_H

/** @file
 *
 * Hyper Text Transport Protocol
 *
 */

FILE_LICENCE ( GPL2_OR_LATER );

struct http_method {
	const char *method;
	const char *content_type;
	char *body;
	size_t body_len;
};

typedef int (*http_callback_t) ( struct xfer_interface *,
				 struct xfer_interface ** );

/** HTTP default port */
#define HTTP_PORT 80

/** HTTPS default port */
#define HTTPS_PORT 443

extern int http_open_filter ( struct xfer_interface *xfer, struct uri *uri,
			      unsigned int default_port,
			      http_callback_t filter,
			      struct http_method *meth  );

#endif /* _GPXE_HTTP_H */
