#include <errno.h>
#include <string.h>

#include "item.h"

#define COUNT_MIN  10
#define LEN_MIN    256

int main (int argc, char *argv[])
{
	char buf[ITEM_POOL_SIZE (COUNT_MIN, LEN_MIN)];
	struct item_pool c;
	struct item *i;

	printf ("allocated %zu bytes for at least %u items and "
		"at least %u bytes line size\n",
		sizeof (buf), COUNT_MIN, LEN_MIN);

	item_pool_init (&c, buf, sizeof (buf));

	for (
		errno = 0;
		(i = item_read (&c, stdin)) != NULL;
		item_pool_reset (&c)
	)
		item_write (stdout, 0, NULL, i);

	if (errno != 0) {
		item_write (stdout, errno, strerror (errno), NULL);
		return 1;
	}

	return 0;
}
