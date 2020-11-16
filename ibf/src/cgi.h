/*
 * FastCGI wrappers and helpers
 *
 * Copyright (c) 2018 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef CGI_H
#define CGI_H

#include <stdarg.h>
#include <stddef.h>

#include <fcgiapp.h>

typedef FCGX_Request cgi_req;

static inline int cgi_init (void)
{
	return FCGX_Init ();
}

static inline int cgi_req_init (cgi_req *o)
{
	return FCGX_InitRequest (o, 0, 0);
}

static inline int cgi_accept (cgi_req *o)
{
	return FCGX_Accept_r (o);
}

static inline int cgi_read (void *data, size_t size, cgi_req *o)
{
	/* assert (size <= INT_MAX) */

	return FCGX_GetStr (data, size, o->in);
}

static inline int cgi_write (const void *data, size_t size, cgi_req *o)
{
	return FCGX_PutStr (data, size, o->out);
}

static inline int cgi_puts (const char *s, cgi_req *o)
{
	return FCGX_PutS (s, o->out);
}

int cgi_puts_escaped (const char *s, cgi_req *o);

static inline int cgi_printf (cgi_req *o, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start (ap, fmt);
	ret = FCGX_VFPrintF (o->out, fmt, ap);
	va_end (ap);

	return ret;
}

char *cgi_uri_escape (const char *s);

char **cgi_parse_vars (const char *query);
void cgi_free_vars (char **list);
const char *cgi_getvar (char **list, const char *name);
char **cgi_load_vars (cgi_req *o, char *buf, size_t size);

#endif  /* CGI_H */
