#include <errno.h>
#include <string.h>

#include "item.h"
#include "limits.h"

int main (int argc, char *argv[])
{
	char buf[ITEM_POOL_SIZE (JANUS_ITEM_COUNT, JANUS_LINE_LENGTH)];
	struct item_pool c;
	struct item *i;

	printf ("allocated %zu bytes for at least %u items and "
		"at least %u bytes line size\n",
		sizeof (buf), JANUS_ITEM_COUNT, JANUS_LINE_LENGTH);

	item_pool_init (&c, buf, sizeof (buf));

	for (
		errno = 0;
		(i = item_read (&c, stdin)) != NULL;
		item_pool_reset (&c)
	)
		item_write (NULL, i, stdout);

	if (errno != 0) {
		item_write (strerror (errno), NULL, stdout);
		return 1;
	}

	return 0;
}
