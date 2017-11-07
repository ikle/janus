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

#include "ipset.h"
#include "group.h"

static int ipset_out (const char *fmt, ...) { return 0; }

static void group_update (void *cookie)
{
	struct group *o = cookie;
	struct ipset_session *s;
	const char *name = "$test", *type = "hash:net";
	struct node *n;
	struct address *a;

	if ((s = ipset_session_init (ipset_out)) == NULL)
		return;

	if (ipset_envopt_parse (s, IPSET_ENV_EXIST, NULL) != 0 ||
	    !ipset_create (s, o->name, type) ||
	    !ipset_create (s, name, type))
		goto error;

	/* NOTE: ignore following ipset errors */

	for (n = o->seq.head; n != NULL; n = n->next)
		for (a = n->seq.head; a != NULL; a = a->next)
			switch (a->type) {
			case ADDRESS_NODE:
				ipset_add_node (s, name, type, &a->node);
				break;
			case ADDRESS_NET:
				ipset_add_net (s, name, type, &a->net);
				break;
			default:
				/* ignore unknown types */
				break;
			}

	ipset_swap (s, name, o->name, type);
	ipset_destroy (s, name, type);
	ipset_commit (s);
error:
	ipset_session_fini (s);
}

struct group *group_alloc (const char *name)
{
	struct group *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		goto no_object;

	if ((o->name = strdup (name)) == NULL)
		goto no_name;

	node_seq_init (&o->seq);
	callout_init (&o->callout, group_update, o);
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

	if ((n = node_alloc_static (&o->callout, from)) == NULL)
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

typedef struct node *
node_alloc_named (struct callout *observer, const char *name);

static int group_load_named (struct group *o,
			     enum node_type type,
			     node_alloc_named *node_alloc,
			     FILE *from)
{
	struct node *n;
	char line [NS_MAXDNAME];
	struct node_seq seq;

	node_seq_init (&seq);

	while (fgets (line, sizeof (line), from) != NULL)
		if ((n = node_alloc (&o->callout, chomp (line))) != NULL)
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
