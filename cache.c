/*
 * Dakota Chip Cache
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <errno.h>
#include <stdlib.h>

#include <dakota/cache.h>
#include <dakota/string.h>

static const char *home;

struct cmdb *dakota_open_tiles (const char *family, const char *mode)
{
	char *path;
	struct cmdb *db;

	if (home == NULL && (home = getenv ("HOME")) == NULL) {
		errno = ENOENT;
		return NULL;
	}

	path = make_string ("%s/.cache/dakota/db/%s.cmdb", home, family);
	if (path == NULL)
		return NULL;

	db = cmdb_open (path, mode);
	free (path);
	return db;
}

struct cmdb *
dakota_open_grid (const char *family, const char *device, const char *mode)
{
	char *path;
	struct cmdb *db;

	if (home == NULL && (home = getenv ("HOME")) == NULL) {
		errno = ENOENT;
		return NULL;
	}

	path = make_string ("%s/.cache/dakota/db/%s-%s.cmdb",
			    home, family, device);
	if (path == NULL)
		return NULL;

	db = cmdb_open (path, mode);
	free (path);
	return db;
}
