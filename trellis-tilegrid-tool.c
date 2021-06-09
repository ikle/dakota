/*
 * Trellis Tile Grid Import
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <err.h>
#include <stdlib.h>

#include <cmdb.h>
#include <dakota/string.h>
#include <json-c/json.h>

static json_object *trellis_open_grid (const char *family, const char *device)
{
	static const char *prefix;
	static const char *trellis;
	char *path;
	json_object *root;

	if (prefix == NULL && (prefix = getenv ("PREFIX")) == NULL)
		prefix = "/usr";

	if (trellis == NULL && (trellis = getenv ("TRELLIS")) == NULL)
		trellis = make_string ("%s/share/trellis/database", prefix);

	if (trellis == NULL)
		return NULL;

	path = make_string ("%s/%s/%s/tilegrid.json", trellis, family, device);
	if (path == NULL)
		return NULL;

	root = json_object_from_file (path);
	free (path);
	return root;
}

static struct cmdb *
dakota_open_grid (const char *family, const char *device, const char *mode)
{
	static const char *home;
	char *path;
	struct cmdb *db;

	if (home == NULL && (home = getenv ("HOME")) == NULL)
		return NULL;

	path = make_string ("%s/.cache/dakota/db/%s-%s.cmdb",
			    home, family, device);
	if (path == NULL)
		return NULL;

	db = cmdb_open (path, mode);
	free (path);
	return db;
}

static const char *json_fetch (json_object *root, const char *name)
{
	json_object *o;

	if (!json_object_object_get_ex (root, name, &o))
		return NULL;

	return json_object_get_string (o);
}

static int import_tile (struct cmdb *db, const char *name, json_object *root)
{
	const char *x = json_fetch (root, "start_frame");
	const char *y = json_fetch (root, "start_bit");

	if (x == NULL || y == NULL)
		return 0;

	return	cmdb_level (db, "tile :", name, NULL) &&
		cmdb_store (db, "x", x) &&
		cmdb_store (db, "y", y);
}

int main (int argc, char *argv[])
{
	struct cmdb *db;
	json_object *root;

	if (argc != 3)
		err (0, "\n\ttrellis-tilegrid <family> <device>");

	if ((db = dakota_open_grid (argv[1], argv[2], "rwx")) == NULL)
		errx (1, "cannot open database");

	if ((root = trellis_open_grid (argv[1], argv[2])) == NULL)
		errx (1, "cannot open tile grid database");

	json_object_object_foreach (root, key, child)
		if (!import_tile (db, key, child))
			warnx ("cannot import tile %s", key);

	json_object_put(root);

	if (!cmdb_close (db))
		errx (1, "cannot commit to database");

	return 0;
}
