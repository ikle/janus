#include <stdlib.h>
#include <string.h>

#include <syslog.h>

#include "chain-hash.h"
#include "cgi.h"
#include "ibf-mapper.h"
#include "pam.h"

/* CGI application */

extern const char _binary_avatar_png_start[];
extern const char _binary_avatar_png_size;

static void show_avatar (cgi_req *o)
{
	cgi_puts ("Status: 200 OK\r\n"
		  "Content-Type: image/png\r\n\r\n", o);

	cgi_write (_binary_avatar_png_start,
		   (size_t) &_binary_avatar_png_size, o);
}

extern const char _binary_login_head_start[], _binary_login_tail_start[];
extern const char _binary_login_head_size,    _binary_login_tail_size;

static void show_login (cgi_req *c, const char *ref)
{
	cgi_puts ("Status: 200 OK\r\n"
		  "Content-Type: text/html; charset=utf-8\r\n\r\n", c);

	cgi_write (_binary_login_head_start,
		   (size_t) &_binary_login_head_size, c);

	if (ref != NULL) {
		cgi_puts ("\t\t<input type=\"hidden\""
			  " name=\"ref\" value=\"", c);
		cgi_puts_escaped (ref, c);
		cgi_puts ("\">\n", c);
	}

	cgi_write (_binary_login_tail_start,
		   (size_t) &_binary_login_tail_size, c);
}

static void do_login (cgi_req *o)
{
	char buf[1024];
	char **list = cgi_load_vars (o, buf, sizeof (buf));
	const char *user = cgi_getvar (list, "user");
	const char *pass = cgi_getvar (list, "pass");
	const char *ref  = cgi_getvar (list, "ref");
	const char *addr = cgi_getvar (o->envp, "REMOTE_ADDR");

	if (addr != NULL && pam_login ("lm", user, pass) == PAM_SUCCESS) {
		ibf_map_user (user, addr);
		cgi_printf (o, "Status: 302 Found\r\nLocation: %s\r\n\r\n",
			    ref == NULL ? "http://ya.ru/" : ref);
	}
	else
		show_login (o, ref);

	cgi_free_vars (list);
}

static void process (cgi_req *o)
{
	const char *path = cgi_getvar (o->envp, "SCRIPT_NAME");

	if (strcmp (path, "/avatar.png") == 0)
		show_avatar (o);
	else if (strcmp (path, "/login") == 0)
		do_login (o);
	else
		cgi_puts ("Status: 404 Not Found\r\n\r\n", o);

}

int main (int argc, char *argv[])
{
	cgi_req o;

	init_chain_hash ();
	openlog ("ibf-login", 0, LOG_DAEMON);

	if (cgi_init () != 0 || cgi_req_init (&o) != 0) {
		syslog (LOG_ERR, "CGI initialization failed");
		return 1;
	}

	while (cgi_accept (&o) == 0)
		process (&o);

	syslog (LOG_INFO, "CGI stopped");
	return 0;
}
