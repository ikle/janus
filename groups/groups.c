/*
 * Janus Groups
 *
 * Copyright (c) 2017 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "groups.h"

struct group *group_alloc (const char *name)
{
	struct group *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		goto no_object;

	if ((o->name = strdup (name)) == NULL)
		goto no_name;

	node_seq_init (&o->seq);
	return o;
no_name:
	free (o);
no_object:
	return NULL;
}

void group_free (struct group *o)
{
	node_seq_fini (&o->seq, node_free_schedule);
	free (o->name);
	free (o);
}

void group_filter_out (struct group *o, enum node_type type)
{
	struct node *n;
	struct node_seq seq;

	node_seq_init (&seq);

	while ((n = node_seq_dequeue (&o->seq)) != NULL)
		if (n->type != type)
			node_seq_enqueue (&seq, n);
		else
			node_free_schedule (n);

	o->seq = seq;
}

int group_load (struct group *o, enum node_type type, FILE *from)
{
	return 0;
}
