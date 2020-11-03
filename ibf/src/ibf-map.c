#include <stdio.h>
#include <syslog.h>

#include <glib.h>

#include "ibf-mapper.h"

int main (int argc, char *argv[])
{
	char line[256];
	int count;
	char **args;

	openlog ("ibf-map", 0, LOG_DAEMON);

	if (setvbuf (stdin, NULL, _IOLBF, 0) != 0) {
		syslog (LOG_ERR, "cannot set line buffered mode");
		return 1;
	}

	while (fgets (line, sizeof (line), stdin) != NULL) {
		if (!g_shell_parse_argv (line, &count, &args, NULL)) {
			syslog (LOG_ERR, "cannot parse line: %s", line);
			printf ("ERR\n");
			continue;
		}

		if (count >= 2) {
			syslog (LOG_INFO, "map %s to %s", args[0], args[1]);
			ibf_map_user (args[0], args[1]);
			printf ("OK\n");
		}
		else {
			syslog (LOG_ERR, "broken line: %s", line);
			printf ("ERR\n");
		}

		fflush(stdout);
		g_strfreev (args);
	}

	return feof (stdin) ? 0 : 1;
}
