#ifndef JANUS_TYPE_H
#define JANUS_TYPE_H  1

enum janus_type {
	JANUS_TYPE_LITERAL,
	JANUS_TYPE_RE,
	JANUS_TYPE_NUMBER,
	JANUS_TYPE_IPV4,
	JANUS_TYPE_IPV4_HOST,
	JANUS_TYPE_IPV4_NET,
	JANUS_TYPE_IPV6,
	JANUS_TYPE_IPV6_HOST,
	JANUS_TYPE_IPV6_NET,
	JANUS_TYPE_EXTERNAL,
};

int janus_type_check (enum janus_type type, const char *arg, const char *data);
int janus_type_numcmp (const char *a, const char *b);

#endif  /* JANUS_TYPE_H */
