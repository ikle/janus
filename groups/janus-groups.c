/*
 * Janus Groups Service
 *
 * Copyright (c) 2017 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>
#include <syslog.h>
#include <unistd.h>

#include "callout.h"
#include "ipset.h"
#include "group.h"

static void load (struct group *o, enum node_type type, const char *path)
{
	FILE *f;

	if ((f = fopen (path, "r")) == NULL)
		return;

	if (!group_load (o, type, f))
		syslog (LOG_WARNING, "group %s list %s is not fully loaded",
			o->name, path);

	fclose (f);
}

static void setup_group (const char *root, const char *name)
{
	struct group *o;
	char path[PATH_MAX];

	if ((o = group_alloc (name)) == NULL) {
		syslog (LOG_ERR, "cannot allocate group %s", name);
		return;
	}

	snprintf (path, sizeof (path), "%s/%s/address", root, name);
	load (o, NODE_STATIC, path);

	snprintf (path, sizeof (path), "%s/%s/domain", root, name);
	load (o, NODE_DOMAIN, path);

	if (node_seq_is_empty (&o->seq)) {
		syslog (LOG_INFO, "ignore empty group %s", name);
		group_free (o);
	}

	/* let the group live on their own lives, no memory leak! */
}

int main (void)
{
	const char *root = "/var/lib/janus/groups";
	FILE *f;
	DIR *d;
	struct dirent *de;

	setlocale (LC_ALL, "");

	if (daemon (0, 0) != 0) {
		perror ("janus-groups");
		return 1;
	}

	openlog ("janus-groups", 0, LOG_DAEMON);

	if ((f = fopen ("/var/run/janus-groups.pid", "w")) == NULL)
		syslog (LOG_WARNING, "cannot create pid file");
	else {
		fprintf (f, "%lu", (unsigned long) getpid ());
		fclose (f);
	}

	callout_sys_init ();
	ipset_load_types ();

	if ((d = opendir (root)) == NULL) {
		syslog (LOG_INFO, "cannot open groups directory, exiting");
		return 0;
	}

	while ((de = readdir (d)) != NULL)
		if (de->d_name[0] != '.')
			setup_group (root, de->d_name);

	closedir (d);

	for (;; sleep (1))
		callout_process ();

	return 0;
}
