/*
 * Dakota Chip Tile
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dakota/tile.h>

struct tile {
	struct cmdb *db;
	char *family;
	char *type;

	struct bitmap *map;
	char error[128];
};

struct tile *tile_alloc (struct cmdb *db, const char *family, const char *type)
{
	struct tile *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	if ((o->map = bitmap_alloc ()) == NULL)
		goto no_map;

	if ((o->family = strdup (family)) == NULL)
		goto no_family;

	if ((o->type = strdup (type)) == NULL)
		goto no_type;

	o->db = db;
	o->error[0] = '\0';
	return o;
no_type:
	free (o->family);
no_family:
	bitmap_free (o->map);
no_map:
	free (o);
	return NULL;
}

void tile_free (struct tile *o)
{
	if (o == NULL)
		return;

	cmdb_close (o->db);
	bitmap_free (o->map);
	free (o);
}

int tile_set_bits (struct tile *o, const unsigned *bits)
{
	return bitmap_add (o->map, bits);
}

static int tile_add_bits (struct tile *o, const char *value, int invert)
{
	unsigned *bits;
	int ok;

	if (strcmp (value, "-") == 0)
		return 1;

	bits = chip_bits_parse (value);

	if (invert)
		chip_bits_invert (bits);

	ok = bitmap_add (o->map, bits);
	free (bits);
	return ok;
}

int tile_set_raw (struct tile *o)
{
	const char *bits;

	if (!cmdb_level (o->db, "family :", o->family, "tile :", o->type, NULL))
		return 0;

	for (
		bits = cmdb_first (o->db, "raw");
		bits != NULL;
		bits = cmdb_next (o->db, "raw", bits)
	)
		if (!tile_add_bits (o, bits, 0))
			return 0;

	return 1;
}

int tile_set_mux (struct tile *o, const char *name, const char *source)
{
	if (!cmdb_level (o->db, "family :", o->family, "tile :", o->type,
				"mux :", name, NULL))
		return 0;

	if ((source = cmdb_first (o->db, source)) == NULL) {
		errno = ENOENT;
		return 0;
	}

	return tile_add_bits (o, source, 0);
}

int tile_set_word (struct tile *o, const char *name, const char *value)
{
	size_t n = strlen (value), i;
	char key[16];
	const char *bits;

	if (!cmdb_level (o->db, "family :", o->family, "tile :", o->type,
				"word :", name, NULL))
		return 0;

	for (i = 0; i < n; ++i) {
		snprintf (key, sizeof (key), "%zu", i);

		if ((bits = cmdb_first (o->db, key)) == NULL) {
			errno = ENOENT;
			return 0;
		}

		if (!tile_add_bits (o, bits, value[n - 1 - i] == '0'))
			return 0;
	}

	return 1;
}

int tile_set_enum (struct tile *o, const char *name, const char *value)
{
	if (!cmdb_level (o->db, "family :", o->family, "tile :", o->type,
				"enum :", name, NULL))
		return 0;

	if ((value = cmdb_first (o->db, value)) == NULL) {
		errno = ENOENT;
		return 0;
	}

	return tile_add_bits (o, value, 0);
}

const struct bitmap *tile_get_bits (const struct tile *o)
{
	return o->map;
}