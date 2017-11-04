/*
 * Internet address helpers
 *
 * Copyright (c) 2017 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <arpa/inet.h>

#include "inet.h"

/*
 * We hard code maximum address length to be in sync with scanf format.
 * (Thus we cannot use INET{,6}_ADDRSTRLEN.)
 */
#define IPV4_LEN  16
#define IPV4_FORMAT  "%15[.0-9]"
#define IPV6_LEN  46
#define IPV6_FORMAT  "%45[.0-9:A-Fa-f]"

int get_ipv4 (const char *from, struct in_addr *to)
{
	return inet_pton (AF_INET, from, to);
}

int get_ipv6 (const char *from, struct in6_addr *to)
{
	return inet_pton (AF_INET6, from, to);
}

int get_ipv4_masked (const char *from, struct ipv4_masked *to)
{
	char addr[IPV4_LEN], tail;

	if (sscanf (from, IPV4_FORMAT "/%u%c",
		    addr, &to->mask, &tail) != 2)
		return 0;

	return get_ipv4 (addr, &to->addr);
}

int get_ipv6_masked (const char *from, struct ipv6_masked *to)
{
	char addr[IPV6_LEN], tail;

	if (sscanf (from, IPV6_FORMAT "/%u%c",
		    addr, &to->mask, &tail) != 2)
		return 0;

	return get_ipv6 (addr, &to->addr);
}

int get_ipv4_range (const char *from, struct ipv4_range *to)
{
	char start[IPV4_LEN], stop[IPV4_LEN], tail;

	if (sscanf (from, IPV4_FORMAT "-" IPV4_FORMAT "%c",
		    start, stop, &tail) != 2)
		return 0;

	return get_ipv4 (start, &to->start) && get_ipv4 (stop, &to->stop);
}

int get_ipv6_range (const char *from, struct ipv6_range *to)
{
	char start[IPV6_LEN], stop[IPV6_LEN], tail;

	if (sscanf (from, IPV6_FORMAT "-" IPV6_FORMAT "%c",
		    start, stop, &tail) != 2)
		return 0;

	return get_ipv6 (start, &to->start) && get_ipv6 (stop, &to->stop);
}
