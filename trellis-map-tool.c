/*
 * Trellis Design Map to Bitmap
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static char *make_str_va (const char *fmt, va_list ap)
{
	va_list aq;
	int size;
	char *s;

	va_copy (aq, ap);
	size = vsnprintf (NULL, 0, fmt, aq) + 1;
	va_end (aq);

	if ((s = malloc (size)) == NULL)
		return NULL;

	vsnprintf (s, size, fmt, ap);
	return s;
}

static char *make_str (const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	s = make_str_va (fmt, ap);
	va_end(ap);

	return s;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cmdb.h>
#include <dakota/bitmap.h>
#include <dakota/tile.h>

#include "trellis-conf.h"

struct ctx {
	struct chip_conf *conf;
	const char *family;
	struct cmdb *tiles, *grid;
	struct bitmap *chip;

	struct tile *tile;  /* ToDo: Create lists of tile to support groups */
	size_t x, y;
};

static int on_device (void *cookie, const char *name)
{
	struct ctx *o = cookie;
	char *path;

	if (o->grid != NULL)
		return chip_error (o->conf, "device defined already");

	if ((path = make_str ("test/%s-%s.cmdb", o->family, name)) == NULL)
		return chip_error (o->conf, "cannot make device path");

	o->grid = cmdb_open (path, "r");
	free (path);

	if (o->grid != NULL)
		return 1;

	return chip_error (o->conf, "cannot open device database");
}

static int on_comment (void *cookie, const char *value)
{
	return 1;
}

static int on_sysconfig (void *cookie, const char *name, const char *value)
{
	return 1;
}

static int on_tile (void *cookie, const char *name)
{
	struct ctx *o = cookie;
	char *type;
	const char *v;

	if (o->grid == NULL)
		return chip_error (o->conf, "device does not defined");

	if ((type = strchr (name, ':')) == NULL)
		return chip_error (o->conf, "cannot parse tile name");

	++type;

	tile_free (o->tile);  /* ToDo: Create list of tiles to support groups */

	if ((o->tile = tile_alloc (o->tiles, type)) == NULL)
		return chip_error (o->conf, "cannot create tile");

	if (!cmdb_level (o->grid, "tile :", name, NULL) ||
	    (v = cmdb_first (o->grid, "x")) == NULL)
		return chip_error (o->conf, "cannot get grid for %s", name);

	o->x = atol (v);

	if ((v = cmdb_first (o->grid, "y")) == NULL)
		return chip_error (o->conf, "cannot get grid for %s", name);

	o->y = atol (v);

	return 1;
}

static int on_raw (void *cookie, unsigned bit)
{
	struct ctx *o = cookie;

	if (o->tile == NULL)
		return chip_error (o->conf, "tile does not defined");

	if (!tile_set_bits (o->tile, &bit))
		return chip_error (o->conf, "cannot apply raw");

	return 1;
}

static int on_arrow (void *cookie, const char *sink, const char *source)
{
	struct ctx *o = cookie;

	if (o->tile == NULL)
		return chip_error (o->conf, "tile does not defined");

	if (!tile_set_mux (o->tile, sink, source))
		return chip_error (o->conf, "cannot apply arrow");

	return 1;
}

static int on_mux (void *cookie, const char *name)
{
	struct ctx *o = cookie;

	return chip_error (o->conf, "unexpected mux entry");
}

static int on_mux_data (void *cookie, const char *source, unsigned *bits)
{
	struct ctx *o = cookie;

	return chip_error (o->conf, "unexpected mux data entry");
}

static int on_word (void *cookie, const char *name, const char *value)
{
	struct ctx *o = cookie;

	if (o->tile == NULL)
		return chip_error (o->conf, "tile does not defined");

	if (!tile_set_word (o->tile, name, value))
		return chip_error (o->conf, "cannot apply word");

	return 1;
}

static int on_word_data (void *cookie, unsigned *bits)
{
	struct ctx *o = cookie;

	return chip_error (o->conf, "unexpected word data entry");
}

static int on_enum (void *cookie, const char *name, const char *value)
{
	struct ctx *o = cookie;

	if (o->tile == NULL)
		return chip_error (o->conf, "tile does not defined");

	if (!tile_set_enum (o->tile, name, value))
		return chip_error (o->conf, "cannot apply enum");

	return 1;
}

static int on_enum_data (void *cookie, const char *key, unsigned *bits)
{
	struct ctx *o = cookie;

	return chip_error (o->conf, "unexpected enum data entry");
}

static int on_bram (void *cookie, const char *name)
{
	return 1;
}

static int on_bram_data (void *cookie, unsigned value)
{
	return 1;
}

static int on_commit (void *cookie)
{
	struct ctx *o = cookie;
	int ok;

	if (o->tile == NULL)
		return 1;

	ok = bitmap_blit (o->chip, o->x, o->y, tile_get_bits (o->tile));

	tile_free (o->tile);
	o->tile = NULL;

	return ok ? 1 : chip_error (o->conf, "cannot blit tile");
}

static const struct chip_action action = {
	.on_device	= on_device,
	.on_comment	= on_comment,
	.on_sysconfig	= on_sysconfig,

	.on_tile	= on_tile,

	.on_raw		= on_raw,
	.on_arrow	= on_arrow,

	.on_mux		= on_mux,
	.on_mux_data	= on_mux_data,

	.on_word	= on_word,
	.on_word_data	= on_word_data,

	.on_enum	= on_enum,
	.on_enum_data	= on_enum_data,

	.on_bram	= on_bram,
	.on_bram_data	= on_bram_data,

	.on_commit	= on_commit,
};

#include <err.h>

int main (int argc, char *argv[])
{
	struct chip_conf c;
	struct ctx o;
	char *path;
	FILE *in;
	int ok;

	if (argc != 4)
		errx (0, "\n\t"
			 "trellis-map <family> <design.trellis> <out.pnm>");

	c.action = &action;
	c.cookie = &o;
	c.error[0] = '\0';

	o.conf   = &c;
	o.family = argv[1];
	o.grid   = NULL;

	o.tile   = NULL;

	if ((path = make_str ("test/%s.cmdb", o.family)) == NULL)
		err (1, "cannot make database path");

	if ((o.tiles = cmdb_open (path, "r")) == NULL)
		errx (1, "cannot open database");

	free (path);

	if ((o.chip = bitmap_alloc ()) == NULL)
		err (1, "cannot create chip bitmap");

	if ((in = fopen (argv[2], "r")) == NULL)
		err (1, "cannot open design file %s", argv[2]);

	ok = trellis_read_conf (&c, in);
	fclose (in);

	if (!ok)
		errx (1, c.error);

	if (!bitmap_export (o.chip, argv[3]))
		err (1, "cannot export bitmap to %s", argv[3]);

	tile_free (o.tile);

	cmdb_close (o.tiles);
	cmdb_close (o.grid);
	bitmap_free (o.chip);

	return 0;
}
