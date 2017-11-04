/*
 * Janus Groups Node
 *
 * Copyright (c) 2017 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"

static struct node *node_alloc (enum node_type type)
{
	struct node *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	o->next = NULL;
	o->type = type;

	address_seq_init (&o->seq);
	o->name = NULL;

	return o;
}

static void node_free (void *cookie)
{
	struct node *o = cookie;

	address_seq_fini (&o->seq, address_free);
	free (o->name);
	free (o);
}

struct node *node_alloc_static (FILE *from)
{
	struct node *o;

	if ((o = node_alloc (NODE_STATIC)) == NULL)
		goto no_object;

	if (!address_seq_load (&o->seq, from))
		goto no_load;

	return o;
no_load:
	node_free (o);
no_object:
	return NULL;
}

static struct node *node_alloc_named (enum node_type type, const char *name)
{
	struct node *o;

	if ((o = node_alloc (type)) == NULL)
		goto no_object;

	if ((o->name = strdup (name)) == NULL)
		goto no_name;

	return o;
no_name:
	node_free (o);
no_object:
	return NULL;

}

struct node *node_alloc_domain (const char *name)
{
	return node_alloc_named (NODE_DOMAIN, name);
}

struct node *node_alloc_zone (const char *name)
{
	return node_alloc_named (NODE_ZONE, name);
}

void node_free_schedule (struct node *o)
{
	callout_init (&o->callout, node_free, NULL);
	callout_schedule (&o->callout, 0);
}

void node_update (struct node *o, struct address_seq *seq)
{
	address_seq_fini (&o->seq, address_free);
	o->seq = *seq;
}
