/*
 * Dakota Common Helpers
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>

#include <dakota/util.h>

#ifndef PREFIX
#define PREFIX		"/usr"
#endif

#ifndef DATADIR
#define DATADIR		PREFIX "/share"
#endif

#ifndef USER_DATADIR
#define USER_DATADIR	".local/share"
#endif

struct shell *dakota_open (const char *category, const char *path)
{
	struct shell *sh;
	const char *home, *fmt;

	if (path[0] != '+')
		return shell_alloc ("%s", path);

	++path;

	home = getenv ("HOME");
	fmt  = "%s/" USER_DATADIR "/dakota/%s/%s";

	if (home != NULL &&
	    (sh = shell_alloc (fmt, home, category, path)) != NULL)
		return sh;

	fmt = DATADIR "/dakota/%s/%s";

	if ((sh = shell_alloc (fmt, category, path)) != NULL)
		return sh;

	return NULL;
}
