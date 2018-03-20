/*
 * PAM login helper
 *
 * Copyright (c) 2018 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include "pam.h"

static int login_cb (int num_msg, const struct pam_message **msg,
		     struct pam_response **resp, void *appdata_ptr)
{
	struct pam_response *reply;

	if(num_msg != 1 || (*msg)[0].msg_style != PAM_PROMPT_ECHO_OFF)
		return PAM_CONV_ERR;

	if ((reply = malloc (sizeof (*reply))) == NULL)  /* num_msg times */
		return PAM_CONV_ERR;

	reply[0].resp = strdup (appdata_ptr);  /* ignore errors */
	reply[0].resp_retcode = 0;  /* unused */

	*resp = reply;
	return PAM_SUCCESS;
}

int pam_login (const char *group, const char *user, const char *pass)
{
	struct pam_conv conv;
	pam_handle_t *pamh;
	int ret;

	if (user == NULL || pass == NULL)
		return PAM_USER_UNKNOWN;

	conv.conv = login_cb;
	conv.appdata_ptr = (void *) pass;

	ret = pam_start (group, user, &conv, &pamh);
	if (ret == PAM_SUCCESS)
		ret = pam_authenticate (pamh, 0);

	if (ret == PAM_SUCCESS)
		ret = pam_acct_mgmt (pamh, 0);  /* authorize */

	(void) pam_end (pamh, ret);
	return ret;
}
