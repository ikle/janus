#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "cgi.h"

static void process (cgi_req *o)
{
	const char *lm   = cgi_getvar (o->envp, "LM");
	const char *host = cgi_getvar (o->envp, "HTTP_HOST");
	const char *uri  = cgi_getvar (o->envp, "REQUEST_URI");
	char *host_e, *uri_e;

	if (lm == NULL || host == NULL || uri == NULL)
		goto no_vars;

	if ((host_e = cgi_uri_escape (host)) == NULL)
		goto no_host;

	if ((uri_e = cgi_uri_escape (uri)) == NULL)
		goto no_uri;

	cgi_printf (o, "Status: 302 Found\r\n"
		       "Location: http://%s/login?ref=http:%%2F%%2F%s/%s\r\n"
		       "\r\n",
		    lm, host_e, uri_e);

	free (uri_e);
	free (host_e);
	return;
no_uri:
	free (host_e);
no_host:
no_vars:
	cgi_puts ("Status: 404 Not Found\r\n\r\n", o);
}

int main (int argc, char *argv[])
{
	cgi_req o;

	openlog ("ibf-gate", 0, LOG_DAEMON);

	if (cgi_init () != 0 || cgi_req_init (&o) != 0) {
		syslog (LOG_ERR, "CGI initialization failed");
		return 1;
	}

	while (cgi_accept (&o) == 0)
		process (&o);

	syslog (LOG_INFO, "CGI stopped");
	return 0;
}
