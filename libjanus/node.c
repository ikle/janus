#include <assert.h>
#include <stdarg.h>
#include <glib.h>

#include "janus.h"

struct janus_node {
	char *template;
	char *config;
};

static struct janus_node *janus_node_alloc_eat (char *template, char *config)
{
	struct janus_node *n;

	assert (template != NULL);
	assert (config   != NULL);

	n = g_malloc (sizeof (*n));

	n->template = template;
	n->config   = config;

	return n;
}

struct janus_node *janus_node_alloc (const char *template, const char *config)
{
	return janus_node_alloc_eat (g_strdup (template), g_strdup (config));
}

void janus_node_free (struct janus_node *n)
{
	if (n == NULL)
		return;

	g_free (n->template);
	g_free (n->config);
	g_free (n);
}

struct janus_node *janus_node_get (const struct janus_node *root,
				   const char *name)
{
	char *template, *config, *name_escaped;

	assert (root != NULL);
	assert (name != NULL);

	template = g_build_filename (root->template, name, NULL);
	if (g_file_test (template, G_FILE_TEST_IS_DIR))
		goto found;

	g_free (template);

	template = g_build_filename (root->template, "node.tag", NULL);
	if (g_file_test (template, G_FILE_TEST_IS_DIR))
		goto found;

	g_free (template);
	return NULL;
found:
	name_escaped = g_uri_escape_string (name, NULL, TRUE);
	config = g_build_filename (root->config, name_escaped, NULL);
	g_free (name_escaped);

	return janus_node_alloc_eat (template, config);
}

int janus_node_exists (const struct janus_node *n)
{
	return g_file_test (n->config, G_FILE_TEST_IS_DIR);
}

char *janus_node_value_get (const struct janus_node *n)
{
	char *path, *value;
	gboolean status;

	assert (n != NULL);

	path = g_build_filename (n->config, "node.val", NULL);
	status = g_file_get_contents (path, &value, NULL, NULL);

	g_free (path);
	return status ? value : NULL;
}

int janus_node_value_set (const struct janus_node *n, const char *value)
{
	char *path;
	gboolean status;

	assert (n != NULL);

	if (g_mkdir_with_parents (n->config, 0777) != 0)
		return 0;

	path = g_build_filename (n->config, "node.val", NULL);
	status = value != NULL ?
		g_file_set_contents (path, value, -1, NULL) :
		g_unlink (path) == 0;

	g_free (path);
	return status;
}

/*
 * High-level interface to operate configuration
 */
static struct janus_node *janus_get_nodev (const struct janus_node *root,
					   va_list ap)
{
	const char *name;
	struct janus_node *node, *last;

	assert (root != NULL);

	for (last = NULL; (name = va_arg (ap, const char *)) != NULL;) {
		node = janus_node_get (last == NULL ? root : last, name);
		janus_node_free (last);

		if (node == NULL)
			return NULL;

		last = node;
	}

	return last;
}

struct janus_node *janus_get_node (const struct janus_node *root, ...)
{
	va_list ap;
	struct janus_node *node;

	va_start (ap, root);
	node = janus_get_nodev (root, ap);
	va_end (ap);

	return node;
}

char *janus_get_value (const struct janus_node *root, ...)
{
	va_list ap;
	struct janus_node *node;
	char *value;

	assert (root != NULL);

	va_start (ap, root);
	node = janus_get_nodev (root, ap);
	va_end (ap);

	if (node == NULL)
		return NULL;

	value = janus_node_value_get (node);
	janus_node_free (node);

	return value;
}

int janus_set_value (const struct janus_node *root, const char *value, ...)
{
	va_list ap;
	struct janus_node *node;
	int status;

	assert (root  != NULL);

	va_start (ap, value);
	node = janus_get_nodev (root, ap);
	va_end (ap);

	if (node == NULL)
		return 0;

	status = janus_node_value_set (node, value);
	janus_node_free (node);

	return status;
}
