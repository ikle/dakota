/*
 * Trellis Tile Data Import
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dakota/cache.h>
#include <dakota/chip-bits.h>

#include "trellis-conf.h"

struct ctx {
	struct chip_conf *conf;
	struct cmdb *db;
	char *tile;
	int i;
};

static int on_device (void *cookie, const char *name)
{
	struct ctx *o = cookie;

	return chip_error (o->conf, "unexpected device entry");
}

static int on_comment (void *cookie, const char *value)
{
	return 1;
}

static int on_sysconfig (void *cookie, const char *name, const char *value)
{
	struct ctx *o = cookie;

	return chip_error (o->conf, "unexpected sysconfig entry");
}

static int on_tile (void *cookie, const char *name)
{
	struct ctx *o = cookie;

	return chip_error (o->conf, "unexpected tile entry");
}

static int on_raw (void *cookie, unsigned bit)
{
	struct ctx *o = cookie;
	char *value;
	int ok;

	if ((value = chip_bits_string (&bit)) == NULL)
		return chip_error (o->conf, "cannot fetch bits");

	ok = cmdb_level (o->db, NULL) && cmdb_store (o->db, "raw", value);
	free (value);
	return ok ? 1 : chip_error (o->conf, "cannot store raw");
}

static int on_arrow (void *cookie, const char *sink, const char *source)
{
	struct ctx *o = cookie;
	int ok;

	ok = cmdb_level (o->db, "tile :", o->tile, "arrow", NULL) &&
	     cmdb_store (o->db, sink, source);

	return ok ? 1 : chip_error (o->conf, "cannot store arrow");
}

static int on_mux (void *cookie, const char *name)
{
	struct ctx *o = cookie;
	int ok;

	ok = cmdb_level (o->db, "tile :", o->tile, "mux :", name, NULL);

	return ok ? 1 : chip_error (o->conf, "cannot store mux");
}

static int on_mux_data (void *cookie, const char *source, unsigned *bits)
{
	struct ctx *o = cookie;
	char *value;
	int ok;

	if ((value = chip_bits_string (bits)) == NULL)
		return chip_error (o->conf, "cannot fetch bits");

	ok = cmdb_store (o->db, source, value);
	free (value);
	return ok ? 1 : chip_error (o->conf, "cannot store mux data");
}

static int on_word (void *cookie, const char *name, const char *value)
{
	struct ctx *o = cookie;
	int ok;

	o->i = strlen (value);

//	printf ("word %s = %s\n", name, value);
	ok = cmdb_level (o->db, "tile :", o->tile, "word :", name, NULL);

	return ok ? 1 : chip_error (o->conf, "cannot store word");
}

static int on_word_data (void *cookie, unsigned *bits)
{
	struct ctx *o = cookie;
	char key[16], *value;
	int ok;

	if (o->i < 0)
		return chip_error (o->conf, "wrong word count");

	snprintf (key, sizeof (key), "%d", --o->i);

	if ((value = chip_bits_string (bits)) == NULL)
		return chip_error (o->conf, "cannot fetch bits");

	ok = cmdb_store (o->db, key, value);
	free (value);

	return ok ? 1 : chip_error (o->conf, "cannot store word data");
}

static int on_enum (void *cookie, const char *name, const char *value)
{
	struct ctx *o = cookie;
	int ok;

//	printf ("enum %s = %s\n", name, value);
	ok = cmdb_level (o->db, "tile :", o->tile, "enum :", name, NULL);

	return ok ? 1 : chip_error (o->conf, "cannot store enum");
}

static int on_enum_data (void *cookie, const char *key, unsigned *bits)
{
	struct ctx *o = cookie;
	char *value;
	int ok;

	if ((value = chip_bits_string (bits)) == NULL)
		return chip_error (o->conf, "cannot fetch bits");

	ok = cmdb_store (o->db, key, value);
	free (value);

	return ok ? 1 : chip_error (o->conf, "cannot store enum data");
}

static int on_bram (void *cookie, const char *name)
{
	struct ctx *o = cookie;

	return chip_error (o->conf, "unexpected bram entry");
}

static int on_bram_data (void *cookie, unsigned value)
{
	struct ctx *o = cookie;

	return chip_error (o->conf, "unexpected bram data entry");
}

static int on_commit (void *cookie)
{
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
	struct ctx o;
	struct chip_conf c;
	FILE *in;
	int ok;

	if (argc != 4)
		errx (0, "\n\t"
			 "trellis-tiledata <family> <type> <in-tile-bits.db>");

	if ((o.db = dakota_open_tiles (argv[1], "rwx")) == NULL)
		errx (1, "cannot open database");

	if ((in = fopen (argv[3], "r")) == NULL)
		err (1, "cannot open tile conf file %s", argv[3]);

	c.action = &action;
	c.cookie = &o;
	c.error[0] = '\0';

	o.conf = &c;
	o.tile = argv[2];

	ok = trellis_read_conf (&c, in);
	fclose (in);

	if (!ok)
		errx (1, c.error);

	if (!cmdb_close (o.db))
		errx (1, "cannot commit to database");

	return 0;
}
