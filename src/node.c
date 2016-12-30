#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"

struct janus_node *janus_node_alloc (struct janus_node *parent,
				     const char *name, int value)
{
	struct janus_node *n;

	assert (parent != NULL);
	assert (name   != NULL);

	if ((n = malloc (sizeof (*n))) == NULL)
		return NULL;

	n->parent = parent;
	n->next   = parent->child;
	n->child  = NULL;

	n->name  = strdup (name);

	n->value = value;
	n->black = 0;
	n->red   = 1;

	parent->child = n;
	return n;
}

static void janus_node_unlink (struct janus_node *n)
{
	struct janus_node **p;

	assert (n         != NULL);
	assert (n->parent != NULL);

	for (p = &n->parent->child; *p != NULL; p = &(*p)->next)
		if (*p == n) {
			*p = n->next;
			return;
		}
}

void janus_node_free (struct janus_node *n)
{
	struct janus_node *p, *next;

	if (n == NULL)
		return;

	janus_node_unlink (n);

	for (p = n->child; p != NULL; p = next) {
		next = p->next;

		janus_node_free (p);
	}

	free (n->name);
	free (n);
}

struct janus_node *janus_node_find (struct janus_node *parent,
				    const char *name)
{
	struct janus_node *p;

	assert (parent != NULL);
	assert (name   != NULL);

	for (p = parent->child; p != NULL; p = p->next)
		if (strcmp (name, p->name) == 0)
			return p;

	errno = ENOENT;
	return NULL;
}

void janus_node_black (struct janus_node *n)
{
	struct janus_node *p;

	assert (n != NULL);

	if (n->red) {
		janus_node_free (n);
		return;
	}

	n->black = 1;

	for (p = n->child; p != NULL; p = p->next)
		janus_node_black (p);
}

int janus_node_commit (struct janus_node *root)
{
	struct janus_node *p, *next;

	assert (root != NULL);

	for (p = root->child; p != NULL; p = next) {
		next = p->next;

		if (p->black) {
			janus_node_free (p);
			continue;
		}

		if (p->red)
			p->red = 0;

		janus_node_commit (p);
	}

	return 0;
}
