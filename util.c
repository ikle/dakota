/*
 * Dakota Common Helpers
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>

#include <dakota/string.h>
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

FILE *dakota_open (const char *category, const char *path)
{
	const char *mode = "r";
	FILE *f;
	const char *home, *fmt;
	char *p;

	if (category == NULL || path[0] != '+')
		return fopen (path, mode);

	++path;

	home = getenv ("HOME");

	if (home != NULL) {
		fmt = "%s/" USER_DATADIR "/dakota/%s/%s";

		if ((p = make_string (fmt, home, category, path)) == NULL)
			return NULL;

		f = fopen (p, mode);
		free (p);

		if (f != NULL)
			return f;
	}

	fmt = DATADIR "/dakota/%s/%s";

	if ((p = make_string (fmt, category, path)) == NULL)
		return NULL;

	f = fopen (p, mode);
	free (p);
	return f;
}
