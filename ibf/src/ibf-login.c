#include <stdlib.h>
#include <string.h>

#include <arpa/ibf/inet.h>
#include <syslog.h>

#include "chain-hash.h"
#include "cgi.h"
#include "conf.h"
#include "ipset.h"
#include "pam.h"

/* configuration helpers */

static int is_user_in_group (const char *user, const char *group)
{
	struct conf *c;
	char buf[64];
	int ret = 0;

	c = conf_clone (NULL, "groups", "user-group", group, "user", NULL);
	if (c == NULL)
		return 0;

	while (conf_get (c, buf, sizeof (buf)))
		if (strcmp (buf, user) == 0) {
			ret = 1;
			break;
		}

	conf_free (c);
	return ret;
}

static int get_ttl (const char *group)
{
	struct conf *c;
	char buf[16];
	int ret = 3600;

	c = conf_clone (NULL, "groups", "user-group", group, "ttl", NULL);
	if (c == NULL)
		goto no_node;

	if (conf_get (c, buf, sizeof (buf)))
		ret = atoi (buf);

	conf_free (c);
no_node:
	return ret;
}

/* ipset helpers */

#ifndef IPSET_V7
static void
ipset_envopt_set(struct ipset_session *s, enum ipset_envopt opt)
{
	ipset_envopt_parse (s, opt, NULL);
}

#define IPSET_OUT_ARG
static int ipset_out (const char *fmt, ...)
#else
#define IPSET_OUT_ARG  , NULL
static
int ipset_out (struct ipset_session *session, void *p, const char *fmt, ...)
#endif
{
	return 0;
}

static void add_group (const char *group, const char *user, struct in_addr *ip)
{
	char hash[28];
	struct ipset_session *s;
	const char *type = "hash:ip";

	if (!is_user_in_group (user, group) ||
	    !get_chain_hash ("groups", group, "user", hash) ||
	    (s = ipset_session_init (ipset_out IPSET_OUT_ARG)) == NULL)
		return;

	ipset_envopt_set (s, IPSET_ENV_EXIST);

	if (!ipset_create (s, hash, type) ||
	    !ipset_set_u32 (s, IPSET_OPT_TIMEOUT, get_ttl (group)))
		goto error;

	ipset_add_node (s, user, type, ip);
error:
	ipset_session_fini (s);
}

static void add_groups (const char *user, const char *addr)
{
	struct in_addr ip;
	struct conf *c;
	char buf[64];

	if (!inet_pton (AF_INET, addr, &ip) ||
	    (c = conf_clone (NULL, "groups", "user-group", NULL)) == NULL)
		return;

	while (conf_get (c, buf, sizeof (buf)))
		add_group (buf, user, &ip);

	conf_free (c);
}

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
		add_groups (user, addr);
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
