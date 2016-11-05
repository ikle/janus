#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "janus.h"

static void print_value (const char *value)
{
	if (value == NULL)
		printf (": value not defined\n");
	else
		printf (": value = %s\n", value);
}

static void show_value (const struct janus_node *root, ...)
{
	va_list ap;
	const char *name;
	struct janus_node *node, *last;
	char *value;

	assert (root != NULL);

	va_start (ap, root);

	for (last = NULL; (name = va_arg (ap, const char *)) != NULL;) {
		if (last != NULL)
			printf (" ");

		printf ("%s", name);

		node = janus_node_get (last == NULL ? root : last, name);
		janus_node_free (last);

		if (node == NULL)
			goto invalid;

		last = node;
	}

	va_end (ap);

	value = janus_node_value_get (last);
	janus_node_free (last);

	print_value (value);
	g_free (value);

	return;
invalid:
	va_end (ap);
	printf (": invalid node\n");
}

int main (int argc, char *argv[])
{
	struct janus_node *root;
	char *value;

	root = janus_node_alloc ("test-template", "test-config");

	show_value (root, "l1", "t1", NULL);
	show_value (root, "l1", "t2", NULL);
	show_value (root, "l2", "t3", NULL);

	printf ("\nHi-level interface:\n\n");

	if (janus_set_value (root, "dolorem ipsum", "l1", "t3", NULL)) {
		value = janus_get_value (root, "l1", "t3", NULL);

		printf ("l1 l3");
		print_value (value);
		g_free (value);

		janus_set_value (root, NULL, "l1", "t3", NULL);
	}
	else
		printf ("E: set value failed\n");

	janus_node_free (root);

	return 0;
}
