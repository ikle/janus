#include <stdio.h>
#include <stdlib.h>

#include "node.h"

static struct janus_node *add (struct janus_node *root,
			       const char *name, int value)
{
	struct janus_node *n = janus_node_alloc (root, name, value);

	if (n == NULL) {
		perror ("add");
		exit (1);
	}

	return n;
}

static void show_prefix (int level, struct janus_node *n, FILE *to)
{
	if (n->red)
		fputc ('+', to);
	else if (n->black)
		fputc ('-', to);
	else
		fputc (' ', to);

	for (; level > 0; --level)
		fputs ("    ", to);
}

static void show_node (int level, struct janus_node *n, FILE *to)
{
	struct janus_node *p;

	for (p = n->child; p != NULL; p = p->next) {
		show_prefix (level, p, to);

		fputs (p->name, to);

		if (p->child != NULL) {
			fputs (" {\n", to);
			show_node (level + 1, p, to);
			show_prefix (level, p, to);
			fputs ("}\n", to);
		}
		else
			fputc ('\n', to);
	}
}

static struct janus_node root;

static void show (void)
{
	show_node (0, &root, stdout);
	fputs ("----\n", stdout);
}

int main (int argc, char *argv[])
{
	struct janus_node *interfaces, *ethernet, *eth0, *eth1, *eth2;
	struct janus_node *system, *hostname;
	struct janus_node *services, *dhcp, *snmp;

	interfaces = add (&root, "interfaces", 0);
	ethernet = add (interfaces, "ethernet", 0);
	eth0 = add (ethernet, "eth0", 1);
	eth1 = add (ethernet, "eth1", 1);
	eth2 = add (ethernet, "eth2", 1);

	system = add (&root, "system", 0);
	hostname = add (system, "hostname", 0);
	add (hostname, "ikle", 1);

	services = add (&root, "services", 0);
	dhcp = add (services, "dhcp", 0);
	snmp = add (services, "snmp", 0);

	show ();

	janus_node_black (system);	show ();
	janus_node_commit (&root);	show ();

	janus_node_black (eth1);	show ();
	janus_node_commit (&root);	show ();

	janus_node_black (ethernet);	show ();
	janus_node_commit (&root);	show ();

	ethernet = add (interfaces, "ethernet", 0);
	eth0 = add (ethernet, "eth0", 1);
	eth1 = add (ethernet, "eth1", 1);
	show ();

	janus_node_black (&root);	show ();
	janus_node_commit (&root);	show ();

	return 0;
}
