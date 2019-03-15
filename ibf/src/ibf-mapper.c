/*
 * IBF user mapping helper
 *
 * Copyright (c) 2018-2019 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include <arpa/ibf/inet.h>

#include "chain-hash.h"
#include "conf.h"
#include "ibf-mapper.h"
#include "ipset.h"

/* configuration helpers */

static int is_user_in_group (const char *user, const char *group)
{
	struct conf *c;
	char buf[64];
	int ret = 0;

	c = conf_clone (NULL, "groups", "user-group", group, "user", NULL);
	if (c == NULL)
		return 0;

	while (conf_get (c, buf, sizeof (buf)))
		if (strcmp (buf, user) == 0) {
			ret = 1;
			break;
		}

	conf_free (c);
	return ret;
}

static int get_ttl (const char *group)
{
	struct conf *c;
	char buf[16];
	int ret = 3600;

	c = conf_clone (NULL, "groups", "user-group", group, "ttl", NULL);
	if (c == NULL)
		goto no_node;

	if (conf_get (c, buf, sizeof (buf)))
		ret = atoi (buf);

	conf_free (c);
no_node:
	return ret;
}

/* ipset helpers */

#ifndef IPSET_V7
static void
ipset_envopt_set(struct ipset_session *s, enum ipset_envopt opt)
{
	ipset_envopt_parse (s, opt, NULL);
}

#define IPSET_OUT_ARG
static int ipset_out (const char *fmt, ...)
#else
#define IPSET_OUT_ARG  , NULL
static
int ipset_out (struct ipset_session *session, void *p, const char *fmt, ...)
#endif
{
	return 0;
}

static void add_group (const char *group, const char *user, struct in_addr *ip)
{
	char hash[28];
	struct ipset_session *s;
	const char *type = "hash:ip";

	if (!is_user_in_group (user, group) ||
	    !get_chain_hash ("groups", group, "user", hash) ||
	    (s = ipset_session_init (ipset_out IPSET_OUT_ARG)) == NULL)
		return;

	ipset_envopt_set (s, IPSET_ENV_EXIST);

	if (!ipset_create (s, hash, type) ||
	    !ipset_set_u32 (s, IPSET_OPT_TIMEOUT, get_ttl (group)))
		goto error;

	ipset_add_node (s, user, type, ip);
error:
	ipset_session_fini (s);
}

void ibf_map_user (const char *user, const char *addr)
{
	struct in_addr ip;
	struct conf *c;
	char buf[64];

	if (!inet_pton (AF_INET, addr, &ip) ||
	    (c = conf_clone (NULL, "groups", "user-group", NULL)) == NULL)
		return;

	while (conf_get (c, buf, sizeof (buf)))
		add_group (buf, user, &ip);

	conf_free (c);
}
