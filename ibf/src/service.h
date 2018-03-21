/*
 * Service helper
 *
 * Copyright (c) 2018 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef SERVICE_H
#define SERVICE_H

int service_start (const char *fmt, ...);
int service_stop (const char *pidfile, unsigned timeout);

#endif  /* SERVICE_H */
