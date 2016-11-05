#ifndef _JANUS_H
#define _JANUS_H  1

#include <stddef.h>

/*
 * Low-level interface to work with Janus nodes
 */
struct janus_node *janus_node_alloc (const char *template, const char *config);
void janus_node_free (struct janus_node *n);

struct janus_node *janus_node_get (const struct janus_node *root,
				   const char *name);

int janus_node_exists (const struct janus_node *n);

char *janus_node_value_get (const struct janus_node *n);
int janus_node_value_set (const struct janus_node *n, const char *value);

/*
 * High-level interface to operate configuration
 */
struct janus_node *janus_get_node (const struct janus_node *root, ...);

char *janus_get_value (const struct janus_node *root, ...);
int janus_set_value (const struct janus_node *root, const char *value, ...);

#endif  /* _JANUS_H */
