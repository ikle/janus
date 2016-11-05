#ifndef _JANUS_H
#define _JANUS_H  1

#include <stddef.h>

struct janus_node *janus_node_alloc (const char *template, const char *config);
void janus_node_free (struct janus_node *n);

struct janus_node *janus_node_get (const struct janus_node *root,
				   const char *name);

int janus_node_exists (const struct janus_node *n);

char *janus_node_value_get (const struct janus_node *n);
int janus_node_value_set (const struct janus_node *n, const char *value);

#endif  /* _JANUS_H */
