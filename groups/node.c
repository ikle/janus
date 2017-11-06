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

#include <netinet/in.h>
#include <netdb.h>

#include "node.h"

static struct node *node_alloc (struct callout *observer, enum node_type type)
{
	struct node *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	o->next = NULL;
	o->type = type;

	address_seq_init (&o->seq);
	o->name = NULL;
	o->observer = observer;

	return o;
}

static void node_free (void *cookie)
{
	struct node *o = cookie;

	address_seq_fini (&o->seq, address_free);
	free (o->name);
	free (o);
}

struct node *node_alloc_static (struct callout *observer, FILE *from)
{
	struct node *o;

	if ((o = node_alloc (observer, NODE_STATIC)) == NULL)
		goto no_object;

	if (!address_seq_load (&o->seq, from))
		goto no_load;

	if (o->observer != NULL)
		callout_schedule (o->observer, 0);

	return o;
no_load:
	node_free (o);
no_object:
	return NULL;
}

static struct node *node_alloc_named (struct callout *observer,
				      enum node_type type, const char *name)
{
	struct node *o;

	if ((o = node_alloc (observer, type)) == NULL)
		goto no_object;

	if ((o->name = strdup (name)) == NULL)
		goto no_name;

	return o;
no_name:
	node_free (o);
no_object:
	return NULL;

}

static void domain_cb (void *cookie)
{
	struct node *o = cookie;
	struct addrinfo hint, *res, *p;
	struct sockaddr_in *s;
	struct address *a;
	struct address_seq seq;

	hint.ai_flags = AI_IDN | AI_IDN_ALLOW_UNASSIGNED;
	hint.ai_family = AF_INET;
	hint.ai_protocol = 0;

	if (getaddrinfo (o->name, NULL, &hint, &res) == 0) {
		address_seq_init (&seq);

		for (p = res; p != NULL; p = p->ai_next)
			if ((a = address_alloc (ADDRESS_NODE)) != NULL) {
				s = (void *) p->ai_addr;
				a->node = s->sin_addr;
				address_seq_enqueue (&seq, a);
			}

		freeaddrinfo (res);
		address_seq_move (&seq, &o->seq, address_free);

		if (o->observer != NULL)
			callout_schedule (o->observer, 0);
	}

	callout_schedule (&o->callout, 60);
}

struct node *node_alloc_domain (struct callout *observer, const char *name)
{
	struct node *o;

	if ((o = node_alloc_named (observer, NODE_DOMAIN, name)) == NULL)
		return NULL;

	callout_init (&o->callout, domain_cb, o);
	callout_schedule (&o->callout, 0);
	return o;
}

struct node *node_alloc_zone (struct callout *observer, const char *name)
{
	return node_alloc_named (observer, NODE_ZONE, name);
}

void node_free_schedule (struct node *o)
{
	callout_init (&o->callout, node_free, NULL);
	callout_schedule (&o->callout, 0);
}
