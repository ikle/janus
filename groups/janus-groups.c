/*
 * Janus Groups Service
 *
 * Copyright (c) 2017 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
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
		perror ("W: list not fully loaded");

	fclose (f);
}

int main (void)
{
	struct group *g;

	callout_sys_init ();
	ipset_load_types ();

	if ((g = group_alloc ("janus-test")) == NULL) {
		perror ("E: cannot create group");
		return 1;
	}

	load (g, NODE_STATIC, "address");
	load (g, NODE_DOMAIN, "domain");

	for (;; sleep (1))
		callout_process ();

	group_free (g);
	return 0;
}
