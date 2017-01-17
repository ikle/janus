#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <regex.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "limits.h"
#include "type.h"

static int get_number (const char *data, int *n)
{
	char *end;
	unsigned long x;

	if (!isdigit (*data))
		return 0;

	x = strtoul (data, &end, 10);

	if (*end != '\0' /* || x > INT_MAX */)
		return 0;

	*n = x;
	return 1;
}

static is_number (const char *data)
{
	int n;

	return get_number (data, &n);
}

static int get_addr (const char *data, size_t len, int domain, void *a)
{
	char buf[INET6_ADDRSTRLEN];

	if (len >= sizeof (buf))
		return 0;

	memcpy (buf, data, len);
	buf[len] = '\0';

	return inet_pton (domain, buf, a) == 1;
}

static int get_addr_mask (const char *data, int domain, void *a, int *m)
{
	char *p;

	if ((p = strchr (data, '/')) == NULL ||
	    !get_addr (data, p - data, domain, a) ||
	    !get_number (p + 1, m))
		return 0;

	if (domain == AF_INET && *m > 32)
		return 0;

	if (domain == AF_INET6 && *m > 128)
		return 0;

	return 1;
}

static int is_ipv4 (const char *data)
{
	struct in_addr a;

	return inet_pton (AF_INET, data, &a) == 1;
}

static int is_ipv4_host (const char *data)
{
	struct in_addr a;
	int m;

	return get_addr_mask (data, AF_INET, &a, &m);
}

static int is_ipv4_net (const char *data)
{
	struct in_addr a;
	int m;

	if (!get_addr_mask (data, AF_INET, &a, &m))
		return 0;

	return (ntohl (a.s_addr) & ~(~0UL << (32 - m))) == 0;
}

static int is_ipv6 (const char *data)
{
	struct in6_addr a;

	return inet_pton (AF_INET6, data, &a) == 1;
}

static int is_ipv6_host (const char *data)
{
	struct in6_addr a;
	int m;

	return get_addr_mask (data, AF_INET6, &a, &m);
}

static int is_ipv6_net (const char *data)
{
	struct in6_addr a;
	int m;

	if (!get_addr_mask (data, AF_INET6, &a, &m))
		return 0;

	if (m >= 96)
		return (ntohl (a.s6_addr32[3]) & ~(~0UL << (128 - m))) == 0;

	if (a.s6_addr32[3] != 0)
		return 0;

	if (m >= 64)
		return (ntohl (a.s6_addr32[2]) & ~(~0UL << (96 - m))) == 0;

	if (a.s6_addr32[2] != 0)
		return 0;

	if (m >= 32)
		return (ntohl (a.s6_addr32[1]) & ~(~0UL << (64 - m))) == 0;

	if (a.s6_addr32[3] != 0)
		return 0;

	return (ntohl (a.s6_addr32[0]) & ~(~0UL << (32 - m))) == 0;
}

static int re_match (const char *re, const char *data)
{
	regex_t reg;
	int ret;

	if (regcomp (&reg, re, REG_EXTENDED | REG_NOSUB) != 0)
		return 0;

	ret = regexec (&reg, data, 0, NULL, 0);
	regfree (&reg);

	return ret == 0;
}

static int file_match (FILE *f, const char *data)
{
	char line[JANUS_LINE_LENGTH];
	size_t len;

	if (f == NULL)
		return 0;

	while ((fgets (line, sizeof (line), f)) != NULL) {
		len = strlen (line);

		if (len > 0 && line[len - 1] == '\n')
			line[len - 1] = '\0';

		if (strcmp (line, data) == 0)
			return 1;
	}

	return 0;
}

static int ext_match (const char *path, const char *data)
{
	FILE *f;
	int ret = 0;

	if (access (path, X_OK) == 0) {
		f = popen (path, "r");
		ret = file_match (f, data);
		pclose (f);
	}
	else if (access (path, R_OK) == 0) {
		f = fopen (path, "r");
		ret = file_match (f, data);
		fclose (f);
	}

	return ret;
}

int type_check (enum janus_type type, const char *arg, const char *data)
{
	switch (type) {
	case JANUS_TYPE_LITERAL:	return strcmp (arg, data) == 0;
	case JANUS_TYPE_RE:		return re_match (arg, data);
	case JANUS_TYPE_NUMBER:		return is_number    (data);
	case JANUS_TYPE_IPV4:		return is_ipv4      (data);
	case JANUS_TYPE_IPV4_HOST:	return is_ipv4_host (data);
	case JANUS_TYPE_IPV4_NET:	return is_ipv4_net  (data);
	case JANUS_TYPE_IPV6:		return is_ipv6      (data);
	case JANUS_TYPE_IPV6_HOST:	return is_ipv6_host (data);
	case JANUS_TYPE_IPV6_NET:	return is_ipv6_net  (data);
	case JANUS_TYPE_EXTERNAL:	return ext_match (arg, data);
	}

	return 0;
}
