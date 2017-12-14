/*
 * Janus Groups Address
 *
 * Copyright (c) 2017 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef JANUS_GROUPS_ADDRESS_H
#define JANUS_GROUPS_ADDRESS_H  1

#include <stdio.h>

#include "inet.h"
#include "seq.h"

enum address_scope {
	ADDRESS_SCOPE_IP,
};

enum address_type {
	ADDRESS_NODE,		/* IP address		*/
	ADDRESS_NET,		/* IP network		*/
	ADDRESS_RANGE,		/* IP address range	*/
};

struct address {
	struct address *next;
	enum address_type type;
	union {
		struct in_addr     node;
		struct ipv4_masked net;
		struct ipv4_range  range;
	};
};

struct address *address_alloc (enum address_type type);
void address_free (struct address *o);

SEQ_DECLARE (address)

struct address *address_parse (enum address_scope scope, const char *from);
int address_seq_load (enum address_scope scope, struct address_seq *seq,
		      FILE *from);

#endif  /* JANUS_GROUPS_ADDRESS_H */
