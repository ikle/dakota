/*
 * Trellis Design Map to Bitmap
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cmdb.h>
#include <dakota/chiplet.h>
#include <dakota/string.h>

#include "trellis-conf.h"

struct ctx {
	struct chip_conf *conf;
	const char *family;
	struct cmdb *grid;
	struct chiplet *chiplet;
	struct bitmap *image;
};

static int on_device (void *cookie, const char *name)
{
	struct ctx *o = cookie;
	char *path;

	if (o->grid != NULL)
		return chip_error (o->conf, "device defined already");

	if ((path = make_string ("test/%s-%s.cmdb", o->family, name)) == NULL)
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
	size_t x, y;

	if (o->grid == NULL)
		return chip_error (o->conf, "device does not defined");

	if ((type = strchr (name, ':')) == NULL)
		return chip_error (o->conf, "cannot parse tile name");

	++type;

	if (!cmdb_level (o->grid, "tile :", name, NULL) ||
	    (v = cmdb_first (o->grid, "x")) == NULL)
		return chip_error (o->conf, "cannot get grid for %s", name);

	x = atol (v);

	if ((v = cmdb_first (o->grid, "y")) == NULL)
		return chip_error (o->conf, "cannot get grid for %s", name);

	y = atol (v);

	if (!chiplet_add (o->chiplet, x, y, type))
		return chip_error (o->conf, "cannot create tile");

	return 1;
}

static int on_raw (void *cookie, unsigned bit)
{
	struct ctx *o = cookie;

	if (!chiplet_set_bits (o->chiplet, &bit))
		return chip_error (o->conf, "cannot apply raw");

	return 1;
}

static int on_arrow (void *cookie, const char *sink, const char *source)
{
	struct ctx *o = cookie;

	if (!chiplet_set_mux (o->chiplet, sink, source))
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

	if (!chiplet_set_word (o->chiplet, name, value))
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

	if (!chiplet_set_enum (o->chiplet, name, value))
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

	ok = chiplet_blit (o->chiplet, o->image);

	chiplet_reset (o->chiplet);

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
	struct cmdb *tiles;
	FILE *in;
	int ok;

	if (argc != 4)
		errx (0, "\n\t"
			 "trellis-map <family> <design.trellis> <out.pnm>");

	o.conf   = &c;
	o.family = argv[1];
	o.grid   = NULL;

	if ((path = make_string ("test/%s.cmdb", o.family)) == NULL)
		err (1, "cannot make database path");

	if ((tiles = cmdb_open (path, "r")) == NULL)
		errx (1, "cannot open database");

	free (path);

	if ((o.chiplet = chiplet_alloc (tiles)) == NULL)
		err (1, "cannot create chiplet");

	if ((o.image = bitmap_alloc ()) == NULL)
		err (1, "cannot create chip bitmap");

	if ((in = fopen (argv[2], "r")) == NULL)
		err (1, "cannot open design file %s", argv[2]);

	c.action = &action;
	c.cookie = &o;
	c.error[0] = '\0';

	ok = trellis_read_conf (&c, in);
	fclose (in);

	if (!ok)
		errx (1, c.error);

	if (!bitmap_export (o.image, argv[3]))
		err (1, "cannot export bitmap to %s", argv[3]);

	cmdb_close (tiles);
	cmdb_close (o.grid);
	chiplet_free (o.chiplet);
	bitmap_free (o.image);

	return 0;
}
