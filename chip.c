/*
 * Dakota Chip
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <dakota/chip.h>
#include <dakota/chiplet.h>

struct chip {
	struct cmdb *grid;
	struct chiplet *chiplet;
	struct bitmap *image;
};

struct chip *chip_alloc (struct cmdb *tiles, struct cmdb *grid)
{
	struct chip *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	o->grid = grid;

	if ((o->chiplet = chiplet_alloc (tiles)) == NULL)
		goto no_chiplet;

	if ((o->image = bitmap_alloc ()) == NULL)
		goto no_bitmap;

	return o;
no_bitmap:
	chiplet_free (o->chiplet);
no_chiplet:
	free (o);
	return NULL;
}

void chip_free (struct chip *o)
{
	if (o == NULL)
		return;

	bitmap_free (o->image);
	chiplet_free (o->chiplet);
	free (o);
}

int chip_add_grid (struct chip *o, struct cmdb *grid)
{
	if (o->grid != NULL) {
		errno = EINVAL;
		return 0;
	}

	o->grid = grid;
	return 1;
}

int chip_add_tile (struct chip *o, const char *name, const char *type)
{
	const char *v;
	size_t x, y;

	if (o->grid == NULL) {
		errno = ENODEV;
		return 0;
	}

	if (!cmdb_level (o->grid, "tile :", name, NULL) ||
	    (v = cmdb_first (o->grid, "x")) == NULL)
		return 0;

	x = atol (v);

	if ((v = cmdb_first (o->grid, "y")) == NULL)
		return 0;

	y = atol (v);

	if (!chiplet_add (o->chiplet, x, y, type))
		return 0;

	return 1;
}

int chip_set_raw (struct chip *o, const unsigned *bits)
{
	return chiplet_set_raw (o->chiplet, bits);
}

int chip_set_mux (struct chip *o, const char *name, const char *source)
{
	return chiplet_set_mux (o->chiplet, name, source);
}

int chip_set_word (struct chip *o, const char *name, const char *value)
{
	return chiplet_set_word (o->chiplet, name, value);
}

int chip_set_enum (struct chip *o, const char *name, const char *value)
{
	return chiplet_set_enum (o->chiplet, name, value);
}

int chip_commit (struct chip *o)
{
	int ok = chiplet_blit (o->chiplet, o->image);

	chiplet_reset (o->chiplet);

	return ok;
}

const struct bitmap *chip_get_bits (const struct chip *o)
{
	return o->image;
}
