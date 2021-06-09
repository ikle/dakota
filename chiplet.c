/*
 * Dakota Chip Tile Group
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>

#include <dakota/chiplet.h>
#include <dakota/tile.h>

/* chiplet unit */

struct unit {
	struct unit *next;
	size_t x, y;
	struct tile *tile;
};

static
struct unit *unit_alloc (struct cmdb *db, size_t x, size_t y, const char *type)
{
	struct unit *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	o->next = NULL;
	o->x    = x;
	o->y    = y;

	if ((o->tile = tile_alloc (db, type)) == NULL)
		goto no_tile;

	return o;
no_tile:
	free (o);
	return NULL;
}

static void unit_free (struct unit *o)
{
	if (o == NULL)
		return;

	tile_free (o->tile);
	free (o);
}

/* chiplet */

struct chiplet {
	struct cmdb *db;
	struct unit *set;
};

struct chiplet *chiplet_alloc (struct cmdb *db)
{
	struct chiplet *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	o->db  = db;
	o->set = NULL;
	return o;
}

void chiplet_reset (struct chiplet *o)
{
	struct unit *u, *next;

	for (u = o->set; u != NULL; u = next) {
		next = u->next;
		unit_free (u);
	}

	o->set = NULL;
}

void chiplet_free (struct chiplet *o)
{
	if (o == NULL)
		return;

	chiplet_reset (o);
	free (o);
}

int chiplet_add (struct chiplet *o, size_t x, size_t y, const char *type)
{
	struct unit *u;

	if ((u = unit_alloc (o->db, x, y, type)) == NULL)
		return 0;

	u->next = o->set;
	o->set = u;
	return 1;
}

int chiplet_set_raw (struct chiplet *o, const unsigned *bits)
{
	struct unit *u;
	int ok = 0;

	for (u = o->set; u != NULL; u = u->next)
		ok |= tile_set_raw (u->tile, bits);

	return ok;
}

int chiplet_set_mux (struct chiplet *o, const char *name, const char *source)
{
	struct unit *u;
	int ok = 0;

	for (u = o->set; u != NULL; u = u->next)
		ok |= tile_set_mux (u->tile, name, source);

	return ok;
}

int chiplet_set_word (struct chiplet *o, const char *name, const char *value)
{
	struct unit *u;
	int ok = 0;

	for (u = o->set; u != NULL; u = u->next)
		ok |= tile_set_word (u->tile, name, value);

	return ok;
}

int chiplet_set_enum (struct chiplet *o, const char *name, const char *value)
{
	struct unit *u;
	int ok = 0;

	for (u = o->set; u != NULL; u = u->next)
		ok |= tile_set_enum (u->tile, name, value);

	return ok;
}

int chiplet_blit (const struct chiplet *o, struct bitmap *image)
{
	struct unit *u;
	int ok = 1;

	for (u = o->set; u != NULL; u = u->next)
		ok &= bitmap_blit (image, u->x, u->y, tile_get_bits (u->tile));

	return ok;
}
