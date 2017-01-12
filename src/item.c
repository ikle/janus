#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "item.h"

/* n must be power of two */
static void *align (void *p, uintptr_t n)
{
	return (void *) (((uintptr_t) p + (n - 1)) & ~(n - 1));
}

/*
 * buffer must be properly aligned to store pointers
 *
 * TODO: check alignment here
 */
void item_pool_init (struct item_pool *c, void *buffer, size_t size)
{
	c->head = buffer;
	c->tail = buffer + size;

	item_pool_reset (c);
}

void item_pool_reset (struct item_pool *c)
{
	c->item = (void *) c->head;
	c->end = c->item->data;
}

static int item_pool_put (struct item_pool *c, int a)
{
	if (c->end >= c->tail) {
		errno = EOVERFLOW;
		return 0;
	}

	*c->end++ = a;
	return 1;
}

static struct item *item_pool_pop (struct item_pool *c)
{
	struct item *i;

	if (!item_pool_put (c, '\0'))
		return NULL;

	i = c->item;
	i->next = NULL;

	c->item = align (c->end, sizeof (i->next));
	c->end = c->item->data;

	return i;
}

struct item *item_read (struct item_pool *c, FILE *in)
{
	struct item *list = NULL, **next = &list;
	int a;
start:
	switch ((a = fgetc (in))) {
	case EOF:  return NULL;
	case '\n': goto eol;
	case ' ':  goto start;
	case '"':  goto quoted;
	default:
		if (!item_pool_put (c, a))
			return NULL;

		goto plain;
	}
plain:
	switch ((a = fgetc (in))) {
	case EOF:  return NULL;
	case '"':  goto quoted;
	case '\n': goto eol;
	case ' ':  goto eoi;
	default:
		if (!item_pool_put (c, a))
			return NULL;

		goto plain;
	}
quoted:
	switch ((a = fgetc (in))) {
	case EOF:  return NULL;
	case '\\': goto escaped;
	case '"':  goto plain;
	default:
		if (!item_pool_put (c, a))
			return NULL;

		goto quoted;
	}
escaped:
	switch ((a = fgetc (in))) {
	case EOF:  return NULL;
	default:
		if (!item_pool_put (c, a))
			return NULL;

		goto quoted;
	}
eoi:
	if ((*next = item_pool_pop (c)) == NULL)
		return NULL;

	next = &(*next)->next;
	goto start;
eol:
	if ((*next = item_pool_pop (c)) == NULL)
		return NULL;

	return list;
}

int write_escaped (const char *p, FILE *out)
{
	if (*p != '\0' && strpbrk (p, " \"") == NULL)
		return fprintf (out, " %s", p) > 0;

	if (fprintf (out, " \"") != 2)
		return 0;

	for (; *p != '\0'; ++p) {
		if (*p == '"' || *p == '\\')
			if (fputc ('\\', out) == EOF)
				return 0;

		if (fputc (*p, out) == EOF)
			return 0;
	}

	return fputc ('"', out) != EOF;
}

int item_write (const char *prefix, struct item *i, FILE *out)
{
	char *p;

	if (prefix != NULL && !write_escaped (prefix, out))
		return 0;

	for (; i != NULL; i = i->next)
		if (!write_escaped (i->data, out))
			return 0;

	return fputc ('\n', out) != EOF;
}
