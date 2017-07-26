#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include "conf.h"

#define ARRAY_SIZE(a)  (sizeof (a) / sizeof ((a)[0]))

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

	c->stack[c->depth = 0] = &c->root;
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

static struct janus_node *current_root (struct janus_conf *c)
{
	return c->stack[c->depth];
}

int janus_conf_set (struct janus_conf *c, struct item *i)
{
	struct janus_node *parent, *n;
	int value;

	assert (c != NULL);

	for (parent = current_root (c); i != NULL; parent = n, i = i->next) {
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

	for (n = current_root (c); i != NULL; i = i->next)
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
		if (p->child != NULL && p->child->value) {
			show_node (p, to);
			continue;
		}

		if (fputs (p->child == NULL ? "leaf" : "group", to) == EOF ||
		    !show_colour (p, to))
			return 0;

		if (p->value && !write_escaped (n->name, to))
			return 0;

		if (!write_escaped (p->name, to) ||
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

	for (n = current_root (c); i != NULL; i = i->next)
		if ((n = janus_node_find (n, i->data)) == NULL)
			return -errno;

	if (!show_node (n, to))
		return -errno;

	return 0;
}

int janus_conf_enter (struct janus_conf *c, struct item *i)
{
	struct janus_node *n;

	assert (c != NULL);

	if (i == NULL)
		return -EINVAL;

	if (c->depth >= (ARRAY_SIZE (c->stack) - 1))
		return -EOVERFLOW;

	for (n = current_root (c); i != NULL; i = i->next)
		if ((n = janus_node_find (n, i->data)) == NULL)
			return -errno;
		else if (n->black)
			return -ENOENT;

	c->stack[++c->depth] = n;
	return 0;
}

int janus_conf_leave (struct janus_conf *c)
{
	assert (c != NULL);

	if (c->depth > 0)
		--c->depth;

	return 0;
}

int janus_conf_home (struct janus_conf *c)
{
	assert (c != NULL);

	c->depth = 0;
	return 0;
}

/* returns nonzero on success */
static int show_node_path (struct janus_node *n, FILE *to)
{
	if (n->parent == NULL)  /* root node */
		return 1;

	return show_node_path (n->parent, to) && write_escaped (n->name, to);
}

int janus_conf_where (struct janus_conf *c, FILE *to)
{
	assert (c != NULL);

	if (fputs ("at", to) == EOF ||
	    !show_node_path (current_root (c), to) ||
	    fputc ('\n', to) == EOF)
		return -errno;

	return 0;
}
