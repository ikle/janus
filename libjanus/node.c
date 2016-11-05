#include <assert.h>
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
