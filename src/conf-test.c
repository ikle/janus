#include <errno.h>
#include <string.h>

#include "conf.h"

static int process (struct janus_conf *c, struct item *i, FILE *to)
{
	if (strcmp (i->data, "set") == 0)
		return janus_conf_set (c, i->next);

	if (strcmp (i->data, "delete") == 0)
		return janus_conf_delete (c, i->next);

	if (strcmp (i->data, "commit") == 0)
		return i->next != NULL ? -EINVAL : janus_conf_commit (c);

	if (strcmp (i->data, "show") == 0)
		return janus_conf_show (c, i->next, to);

	if (strcmp (i->data, "enter") == 0)
		return janus_conf_enter (c, i->next);

	if (strcmp (i->data, "leave") == 0)
		return i->next != NULL ? -EINVAL : janus_conf_leave (c);

	if (strcmp (i->data, "home") == 0)
		return i->next != NULL ? -EINVAL : janus_conf_home (c);

	if (strcmp (i->data, "where") == 0)
		return i->next != NULL ? -EINVAL : janus_conf_where (c, to);

	return -EINVAL;
}

static void status (FILE *to)
{
	fprintf (to, "status %d", errno);
	write_escaped (strerror (errno), to);
	fputc ('\n', to);
}

int main (int argc, char *argv[])
{
	char path[256];
	struct janus_conf c;

	char buf[ITEM_POOL_SIZE (JANUS_ITEM_COUNT, JANUS_LINE_LENGTH)];
	struct item_pool pool;
	struct item *i;

	FILE *from = stdin, *to = stdout;

	janus_conf_init (&c, "template", path, sizeof (path));
	item_pool_init (&pool, buf, sizeof (buf));

	for (
		errno = 0;
		!ferror (to) && (i = item_read (&pool, from)) != NULL;
		status (to), errno = 0, item_pool_reset (&pool)
	)
		errno = -process (&c, i, to);

	return ferror (to) || errno != 0;
}
