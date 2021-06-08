/*
 * Trellis Tile Grid Import
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <err.h>

#include <cmdb.h>
#include <json-c/json.h>

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
		err (0, "\n\ttrellis-tilegrid <out-cmdb> <in-json>");

	if ((db = cmdb_open (argv[1], "rwx")) == NULL)
		errx (1, "cannot open database");

	if ((root = json_object_from_file (argv[2])) == NULL)
		errx (1, "cannot open tile grid database");

	json_object_object_foreach (root, key, child)
		if (!import_tile (db, key, child))
			warnx ("cannot import tile %s", key);

	json_object_put(root);

	if (!cmdb_close (db))
		errx (1, "cannot commit to database");

	return 0;
}
