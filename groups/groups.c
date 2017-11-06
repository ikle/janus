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
#include <arpa/nameser.h>

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

static void group_filter_out (struct group *o, enum node_type type)
{
	struct node *n;
	struct node_seq seq;

	node_seq_init (&seq);

	while ((n = node_seq_dequeue (&o->seq)) != NULL)
		if (n->type != type)
			node_seq_enqueue (&seq, n);
		else
			node_free_schedule (n);

	node_seq_move (&seq, &o->seq, node_free_schedule);
}

static int group_load_static (struct group *o, FILE *from)
{
	struct node *n;

	if ((n = node_alloc_static (from)) == NULL)
		return 0;

	group_filter_out (o, NODE_STATIC);
	node_seq_enqueue (&o->seq, n);
	return 1;
}

static char *chomp (char *line)
{
	char *p;

	if ((p = strchr (line, '\n')) != NULL)
		*p = '\0';

	return line;
}

static int group_load_named (struct group *o,
			     enum node_type type,
			     struct node *(*node_alloc) (const char *name),
			     FILE *from)
{
	struct node *n;
	char line [NS_MAXDNAME];
	struct node_seq seq;

	node_seq_init (&seq);

	while (fgets (line, sizeof (line), from) != NULL)
		if ((n = node_alloc (chomp (line))) != NULL)
			node_seq_enqueue (&seq, n);

	if (ferror (from)) {
		node_seq_fini (&seq, node_free_schedule);
		return 0;
	}

	group_filter_out (o, type);

	while ((n = node_seq_dequeue (&seq)) != NULL)
		node_seq_enqueue (&o->seq, n);

	return 1;
}

int group_load (struct group *o, enum node_type type, FILE *from)
{
	switch (type) {
	case NODE_STATIC:
		return group_load_static (o, from);
	case NODE_DOMAIN:
		return group_load_named (o, type, node_alloc_domain, from);
	case NODE_ZONE:
		return group_load_named (o, type, node_alloc_zone, from);
	}

	errno = EINVAL;
	return 0;
}
