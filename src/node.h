#ifndef JANUS_NODE_H
#define JANUS_NODE_H  1

#include <stddef.h>

struct janus_node {
	struct janus_node *parent, *next, *child;
	char *name;

	unsigned value :1;
	unsigned black :1;
	unsigned red   :1;
};

struct janus_node *janus_node_alloc (struct janus_node *parent,
				     const char *name, int value);

void janus_node_free (struct janus_node *n);

struct janus_node *janus_node_find (struct janus_node *parent,
				    const char *name);

void janus_node_black (struct janus_node *n);
int janus_node_commit (struct janus_node *root);

#endif  /* JANUS_NODE_H */
