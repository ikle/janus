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
	int i;

	if (num_msg <= 0 || num_msg > PAM_MAX_NUM_MSG ||
	    (reply = calloc (num_msg, sizeof (*reply))) == NULL)
		return PAM_CONV_ERR;

	for (i = 0; i < num_msg; ++i)
		switch ((*msg)[i].msg_style) {
		case PAM_PROMPT_ECHO_OFF:
			/* ignore errors */
			reply[i].resp = strdup (appdata_ptr);
			break;
		default:
			reply[i].resp = strdup ("");
			break;
		}

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
