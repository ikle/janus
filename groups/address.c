/*
 * Janus Groups Address
 *
 * Copyright (c) 2017 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "address.h"
#include "inet.h"
#include "ipv4-expand.h"

struct address *address_alloc (enum address_type type)
{
	struct address *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	o->next = NULL;
	o->type = type;
	return o;
}

struct address *address_parse (enum address_scope scope, const char *from)
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

static int add_net (struct ipv4_masked *o, void *cookie)
{
	struct address_seq *seq = cookie;
	struct address *a;

	if ((a = address_alloc (ADDRESS_NET)) == NULL)
		return 0;

	a->net = *o;
	address_seq_enqueue (seq, a);
	return 1;
}

int address_seq_load (enum address_scope scope, struct address_seq *seq,
		      FILE *from)
{
	char line[INET_ADDRSTRLEN * 2];
	struct address *o;

	while (fgets (line, sizeof (line), from) != NULL)
		if ((o = address_parse (scope, chomp (line))) != NULL) {
			if (o->type == ADDRESS_RANGE) {
				ipv4_range_expand (&o->range, add_net, seq);
				address_free (o);
			}
			else
				address_seq_enqueue (seq, o);
		}

	return !ferror (from);
}
