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

#include "ipv4-expand.h"
#include "groups.h"

struct address *address_alloc (enum address_type type)
{
	struct address *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	o->next = NULL;
	o->type = type;
	return o;
}

struct address *address_parse (const char *from)
{
	struct address *o;

	if ((o = address_alloc (ADDRESS_NODE)) == NULL)
		return NULL;

	if (get_ipv4_masked (from, &o->net)) {
		o->type = ADDRESS_NET;
		return o;
	}

	if (get_ipv4_range (from, &o->range)) {
		o->type = ADDRESS_RANGE;
		return o;
	}

	if (get_ipv4 (from, &o->node)) {
		o->type = ADDRESS_NODE;
		return o;
	}

	address_free (o);
	errno = EINVAL;
	return NULL;
}

void address_free (struct address *o)
{
	free (o);
}

static char *chomp (char *line)
{
	char *p;

	if ((p = strchr (line, '\n')) != NULL)
		*p = '\0';

	return line;
}

static int address_seq_load (struct address_seq *seq, FILE *from)
{
	char line[INET_ADDRSTRLEN * 2];
	struct address *o;

	while (fgets (line, sizeof (line), from) != NULL)
		if ((o = address_parse (chomp (line))) != NULL)
			address_seq_enqueue (seq, o);

	return !ferror (from);
}

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
