/*
 * Janus Groups Node
 *
 * Copyright (c) 2017 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef JANUS_GROUPS_NODE_H
#define JANUS_GROUPS_NODE_H  1

#include <stdio.h>

#include "address.h"
#include "callout.h"
#include "seq.h"

enum node_type {
	NODE_STATIC,
	NODE_DOMAIN,
	NODE_ZONE,
};

struct node {
	struct node *next;
	enum node_type type;
	struct address_seq seq;
	char *name;
	time_t ttl;
	struct callout callout;
};

struct node *node_alloc_static (FILE *from);
struct node *node_alloc_domain (const char *name);
struct node *node_alloc_zone (const char *name);
void node_free_schedule (struct node *o);

SEQ_DECLARE (node)

#endif  /* JANUS_GROUPS_NODE_H */
