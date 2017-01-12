#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include "conf.h"

static char *buf_putc (char *head, char *tail, int a)
{
	if (head < tail)
		*head = a;

	return head + 1;
}

static char *buf_puts (char *head, char *tail, const char *string)
{
	for (; *string != '\0'; ++string)
		head = buf_putc (head, tail, *string);

	if (head < tail)
		*head = '\0';

	return head;
}

static char *buf_put_template (char *head, char *tail,
			       const struct janus_node *n)
{
	assert (head != NULL);
	assert (n    != NULL);

	if (n->parent == NULL)  /* root node */
		return head;

	head = buf_put_template (head, tail, n->parent);
	head = buf_putc (head, tail, '/');
	head = buf_puts (head, tail, n->value ? "node.tag" : n->name);

	return head;
}

void janus_conf_init (struct janus_conf *c, const char *template /* path */,
		 void *buffer, size_t size)
{
	assert (c        != NULL);
	assert (template != NULL);
	assert (buffer   != NULL);

	c->root.parent = NULL;  /* root node indicator */
	c->root.child  = NULL;

	c->head = buffer;
	c->tail = c->head + size;
	c->end  = buf_puts (c->head, c->tail, template);
}

/*
 * negative -- error: invalid node or buffer overflow
 *  0 -- simple node
 *  1 -- value node
 */
static int is_value_node (struct janus_conf *c, struct janus_node *parent,
			  const char *name)
{
	char *base, *p;

	base = buf_put_template (c->end, c->tail, parent);
	base = buf_putc (base, c->tail, '/');

	p = buf_puts (base, c->tail, "node.tag");
	if (p >= c->tail)
		return -EOVERFLOW;

	if (access (c->head, F_OK) == 0)
		return 1;

	p = buf_puts (base, c->tail, name);
	if (p >= c->tail)
		return -EOVERFLOW;

	if (access (c->head, F_OK) == 0)
		return 0;

	return -ENOENT;
}

int janus_conf_set (struct janus_conf *c, struct item *i)
{
	struct janus_node *parent, *n;
	int value;

	assert (c != NULL);

	for (parent = &c->root; i != NULL; parent = n, i = i->next) {
		if ((n = janus_node_find (parent, i->data)) != NULL) {
			n->black = 0;
			continue;
		}

		if ((value = is_value_node (c, parent, i->data)) < 0)
			return value;

		n = janus_node_alloc (parent, i->data, value);
		if (n == NULL)
			return -errno;
	}

	return 0;
}

int janus_conf_delete (struct janus_conf *c, struct item *i)
{
	struct janus_node *n;

	assert (c != NULL);

	for (n = &c->root; i != NULL; i = i->next)
		if ((n = janus_node_find (n, i->data)) == NULL)
			return -errno;

	janus_node_black (n);
	return 0;
}

int janus_conf_commit (struct janus_conf *c)
{
	assert (c != NULL);

	janus_node_commit (&c->root);
	return 0;
}

/* show_* helpers return nonzero on succes */
static int show_colour (struct janus_node *n, FILE *to)
{
	return fputs (n->red ? " +" : n->black ? " -" : " \" \"", to) != EOF;
}

static int show_node (struct janus_node *n, FILE *to)
{
	struct janus_node *p;

	for (p = n->child; p != NULL; p = p->next) {
		if (fputs (p->child == NULL ? "leaf" : "group", to) == EOF ||
		    !show_colour (p, to) ||
		    !write_escaped (p->name, to) ||
		    fputc ('\n', to) == EOF)
			return 0;

		if (p->child != NULL)
			if (!show_node (p, to) ||
			    fputs ("end", to) == EOF ||
			    !show_colour (p, to) ||
			    fputc ('\n', to) == EOF)
				return 0;
	}

	return 1;
}

int janus_conf_show (struct janus_conf *c, struct item *i, FILE *to)
{
	struct janus_node *n;

	assert (c != NULL);

	for (n = &c->root; i != NULL; i = i->next)
		if ((n = janus_node_find (n, i->data)) == NULL)
			return -errno;

	if (!show_node (n, to))
		return -errno;

	return 0;
}
