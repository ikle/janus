/*
 * Janus Groups
 *
 * Copyright (c) 2017 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef JANUS_GROUP_H
#define JANUS_GROUP_H  1

#include <stdio.h>

#include "callout.h"
#include "node.h"
#include "seq.h"

struct group {
	char *name;
	struct node_seq seq;
	struct callout callout;
};

struct group *group_alloc (const char *name);
void group_free (struct group *o);

int group_load (struct group *o, enum node_type type, FILE *from);

#endif  /* JANUS_GROUP_H */
