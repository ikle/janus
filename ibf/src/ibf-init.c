#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>

#include "conf.h"
#include "service.h"

static void cfg_add_base (FILE *to, const char *name, const char *user,
			  int port)
{
	fprintf (
		to,
		"server.port           = %3$d\n"
		"server.username       = \"%2$s\"\n"
		"server.groupname      = \"www-data\"\n"
		"server.pid-file       = \"/var/run/ibf/%1$s.pid\"\n"
		"server.document-root  = \"/var/run/ibf\"\n"
		"\n",
		name, user, port
	);
}

static void cfg_add_fgci (FILE *to, const char *name)
{
	fprintf (
		to,
		"fastcgi.server =  (\n"
		"  \"/\" => (\n"
		"    \"%1$s\" => (\n"
		"      \"socket\"      => \"/var/run/ibf/%1$s.sock\",\n"
		"      \"bin-path\"    => \"/usr/sbin/%1$s\",\n"
		"      \"check-local\" => \"disable\",\n"
		"      \"max-procs\"   => 1\n"
		"    )\n"
		"  )\n"
		")\n",
		name
	);
}

static int cfg_gate (const char *pub, int pub_port)
{
	FILE *to;

	if ((to = fopen ("/var/run/ibf/ibf-gate.conf", "w")) == NULL)
		return 0;

	fprintf (to, "server.bind           = \"127.0.0.1\"\n");
	cfg_add_base (to, "ibf-gate", "nobody", 2);

	fprintf (
		to,
		"server.modules = ( \"mod_fastcgi\", \"mod_setenv\" )\n\n"
		"setenv.add-environment = ( \"LM\" => \"%s:%d\" )\n\n",
		pub, pub_port
	);

	cfg_add_fgci (to, "ibf-gate");

	fclose (to);
	return 1;
}

static int cfg_login (int port)
{
	FILE *to;

	if ((to = fopen ("/var/run/ibf/ibf-login.conf", "w")) == NULL)
		return 0;

	cfg_add_base (to, "ibf-login", "root", port);
	fprintf (to, "server.modules = ( \"mod_fastcgi\" )\n\n");
	cfg_add_fgci (to, "ibf-login");

	fclose (to);
	return 1;
}

static int stop (void)
{
	return service_stop ("/var/run/ibf/ibf-gate.pid",  5) &&
	       service_stop ("/var/run/ibf/ibf-login.pid", 5);
}

static int get_port (void)
{
	struct conf *c;
	char buf [16];
	int port;

	if ((c = conf_clone (NULL, "service", "login", "port", NULL)) == NULL)
		return 0;

	if (!conf_get (c, buf, sizeof (buf)) ||
	    (port = atoi (buf)) < 1 || port > 65535)
		port = 0;;

	conf_free (c);
	return port;
}

static int get_pub (void *buf, size_t size)
{
	struct conf *c;
	int ret;

	c = conf_clone (NULL, "service", "login", "public-address", NULL);
	if (c == NULL)
		return 0;

	ret = conf_get (c, buf, size);
	conf_free (c);
	return ret;
}

static int start (void)
{
	int port;
	char pub [256];

	if ((port = get_port ()) == 0) {
		fprintf (stderr, "ibf login: port required\n");
		return 0;
	}

	if (!get_pub (pub, sizeof (pub))) {
		fprintf (stderr, "ibf login: public-address required\n");
		return 0;
	}

	(void) stop ();

	if (!cfg_gate (pub, port) || !cfg_login (port))
		return 0;

	if (!service_start ("lighttpd -f /var/run/ibf/ibf-gate.conf") ||
	    !service_start ("lighttpd -f /var/run/ibf/ibf-login.conf")) {
		(void) stop ();
		return 0;
	}

	return 1;
}

int main (int argc, char *argv[])
{
	int ret = 0;

	(void) mkdir ("/var/run/ibf", 0775);
	(void) chown ("/var/run/ibf", -1, 33 /* www-data */);

	if (argc != 2)
		return 1;

	ret = strcmp (argv[1], "stop")  == 0 ? stop ()  :
	      strcmp (argv[1], "start") == 0 ? start () :
	      0;

	return ret ? 0 : 1;
}
