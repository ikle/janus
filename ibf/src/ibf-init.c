#include <stdio.h>

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

static int cfg_gate (int port, const char *pub)
{
	FILE *to;

	if ((to = fopen ("ibf-gate.conf", "w")) == NULL)
		return 0;

	cfg_add_base (to, "ibf-gate", "nobody", port);

	fprintf (
		to,
		"server.modules = ( \"mod_fastcgi\", \"mod_setenv\" )\n\n"
		"setenv.add-environment = ( \"LM\" => \"%s\" )\n\n",
		pub
	);

	cfg_add_fgci (to, "ibf-gate");

	fclose (to);
	return 1;
}

static int cfg_login (int port)
{
	FILE *to;

	if ((to = fopen ("ibf-login.conf", "w")) == NULL)
		return 0;

	cfg_add_base (to, "ibf-login", "root", port);
	fprintf (to, "server.modules = ( \"mod_fastcgi\" )\n\n");
	cfg_add_fgci (to, "ibf-login");

	fclose (to);
	return 1;
}

int main (int agc, char *argv[])
{
	cfg_gate  (1026, "lm.local");
	cfg_login (1025);

	return 0;
}
