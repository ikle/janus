#ifndef _JANUS_ITEM_H
#define _JANUS_ITEM_H  1

#include <stddef.h>
#include <stdio.h>

struct item {
	struct item *next;
	char data[1];		/* null terminated string */
};

struct item_pool {
	char *end;		/* end of data  */
	struct item *item;	/* current item */
	char *head, *tail;	/* buffer       */
};

/* next pointer + alignment with empty string ('\0') */
#define ITEM_POOL_AUX_MAX  (sizeof (void *) * 2 - 1)

/* (count - 1) spaces + LF */
#define ITEM_POOL_SEP_MIN(count)  (count)

#define ITEM_POOL_SIZE(count, len)  ((count) * ITEM_POOL_AUX_MAX + (len) - \
				     ITEM_POOL_SEP_MIN (count))

/* buffer must be properly aligned to store pointers */
void item_pool_init (struct item_pool *c, void *buffer, size_t size);
void item_pool_reset (struct item_pool *c);

struct item *item_read (struct item_pool *c, FILE *in);

/* returns nonzero on success */
int write_escaped (const char *p, FILE *out);
int item_write (const char *prefix, struct item *i, FILE *out);

#endif  /* _JANUS_IBUF_H */
