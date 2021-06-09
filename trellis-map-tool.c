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
#include <dakota/chip.h>
#include <dakota/string.h>

#include "trellis-conf.h"

struct ctx {
	struct chip_conf *conf;
	const char *family;
	struct cmdb *tiles, *grid;
	struct chip *chip;
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

	if (o->grid == NULL)
		return chip_error (o->conf, "cannot open device database");

	if (!chip_add_grid (o->chip, o->grid))
		return chip_error (o->conf, "cannot assign device to chip");

	return 1;
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

	if (o->grid == NULL)
		return chip_error (o->conf, "device does not defined");

	if ((type = strchr (name, ':')) == NULL)
		return chip_error (o->conf, "cannot parse tile name");

	++type;

	if (!chip_add_tile (o->chip, name, type))
		return chip_error (o->conf, "cannot add tile %s", name);

	return 1;
}

static int on_raw (void *cookie, unsigned bit)
{
	struct ctx *o = cookie;

	if (!chip_set_raw (o->chip, &bit))
		return chip_error (o->conf, "cannot apply raw");

	return 1;
}

static int on_arrow (void *cookie, const char *sink, const char *source)
{
	struct ctx *o = cookie;

	if (!chip_set_mux (o->chip, sink, source))
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

	if (!chip_set_word (o->chip, name, value))
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

	if (!chip_set_enum (o->chip, name, value))
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

	if (!chip_commit (o->chip))
		chip_error (o->conf, "cannot commit changes");

	return 1;
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

	o.conf   = &c;
	o.family = argv[1];
	o.grid   = NULL;

	if ((path = make_string ("test/%s.cmdb", o.family)) == NULL)
		err (1, "cannot make database path");

	if ((o.tiles = cmdb_open (path, "r")) == NULL)
		errx (1, "cannot open database");

	free (path);

	if ((o.chip = chip_alloc (o.tiles, NULL)) == NULL)
		err (1, "cannot create chip");

	if ((in = fopen (argv[2], "r")) == NULL)
		err (1, "cannot open design file %s", argv[2]);

	c.action = &action;
	c.cookie = &o;
	c.error[0] = '\0';

	ok = trellis_read_conf (&c, in);
	fclose (in);

	if (!ok)
		errx (1, c.error);

	if (!bitmap_export (chip_get_bits (o.chip), argv[3]))
		err (1, "cannot export bitmap to %s", argv[3]);

	cmdb_close (o.tiles);
	cmdb_close (o.grid);
	chip_free (o.chip);

	return 0;
}
