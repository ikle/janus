#ifndef _JANUS_CONF_H
#define _JANUS_CONF_H  1

#include "item.h"
#include "limits.h"
#include "node.h"

struct janus_conf {
	struct janus_node root;

	/* template path buffer: end points to end of template root path */
	char *head, *end, *tail;

	int depth;
	struct janus_node *stack[JANUS_ITEM_COUNT];
};

void janus_conf_init (struct janus_conf *c, const char *template /* path */,
		      void *buffer, size_t size);

int janus_conf_set    (struct janus_conf *c, struct item *i);
int janus_conf_delete (struct janus_conf *c, struct item *i);
int janus_conf_commit (struct janus_conf *c);

int janus_conf_show (struct janus_conf *c, struct item *i, FILE *to);

int janus_conf_enter (struct janus_conf *c, struct item *i);
int janus_conf_leave (struct janus_conf *c);
int janus_conf_home  (struct janus_conf *c);
int janus_conf_where (struct janus_conf *c, FILE *to);

#endif  /* _JANUS_CONF_H */
