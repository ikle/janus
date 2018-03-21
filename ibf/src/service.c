/*
 * Service helper
 *
 * Copyright (c) 2018 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

int service_startv (const char *fmt, va_list ap)
{
	int len;
	char *cmd;
	int ret;

	len = vsnprintf (NULL, 0, fmt, ap) + 1;

	if ((cmd = malloc (len)) == NULL)
		return 0;

	vsnprintf (cmd, len, fmt, ap);
	ret = system (cmd);
	free (cmd);

	return ret == -1 ? 0 : WEXITSTATUS (ret) == 0;
}

int service_start (const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start (ap, fmt);
	ret = service_startv (fmt, ap);
	va_end (ap);
	return ret;
}

int service_stop (const char *pidfile, unsigned timeout)
{
	FILE *f;
	char buf[16];
	long pid;

	if ((f = fopen (pidfile, "r")) == NULL)
		return 0;

	if (fgets (buf, sizeof (buf), f) == NULL)
		goto no_read;

	fclose (f);

	if ((pid = atol (buf)) <= 0)
		goto no_pid;

	for (; timeout-- > 0; sleep (1))
		if (kill (pid, SIGTERM) == -1)
			return errno == ESRCH ? 1 : 0;

	return kill (pid, SIGKILL) == 0;
no_read:
	fclose (f);
	return 0;
no_pid:
	errno = EINVAL;
	return 0;
}
