/*
 * Trellis Config Parser Test
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>

#include "trellis-conf.h"

/* Global */

struct ctx {
	size_t bram_max;
};

static int on_device (void *cookie, const char *name)
{
	printf ("device %s\n", name);
	return 1;
}

static int on_comment (void *cookie, const char *value)
{
	printf ("comment = %s\n", value);
	return 1;
}

static int on_sysconfig (void *cookie, const char *name, const char *value)
{
	printf ("sysconfig %s = %s\n", name, value);
	return 1;
}

/* Tile */

static int on_tile (void *cookie, const char *name)
{
	printf ("tile %s\n", name);
	return 1;
}

static int on_arc (void *cookie, const char *sink, const char *source)
{
	printf ("\tarc %s -> %s\n", source, sink);
	return 1;
}

static int on_word (void *cookie, const char *name, const char *value)
{
	printf ("\tword %s = %s\n", name, value);
	return 1;
}

static int on_enum (void *cookie, const char *name, const char *value)
{
	printf ("\tenum %s = %s\n", name, value);
	return 1;
}

static int on_raw (void *cookie, const char *value)
{
	printf ("\traw = %s\n", value);
	return 1;
}

/* BRAM */

static int on_bram (void *cookie, unsigned index)
{
	struct ctx *o = cookie;

	printf ("bram %u", index);
	o->bram_max = 0;
	return 1;
}

static int on_data (void *cookie, unsigned bram, size_t i, unsigned value)
{
	struct ctx *o = cookie;
	const char *sep;

	sep = (i & 15) == 0 ? "\n\t" : (i & 15) == 8 ? "  " : " ";
	printf ("%s%03x", sep, value);
	o->bram_max = i;
	return 1;
}

/* Tile, Tile Group or BRAM */

static int on_commit (void *cookie)
{
	struct ctx *o = cookie;

	if (o->bram_max > 0) {
		printf ("\n");
		o->bram_max = 0;
	}

	printf ("\n");
	return 1;
}

static const struct config_action action = {
	.on_device	= on_device,
	.on_comment	= on_comment,
	.on_sysconfig	= on_sysconfig,

	.on_tile	= on_tile,
	.on_arc		= on_arc,
	.on_word	= on_word,
	.on_enum	= on_enum,
	.on_raw		= on_raw,

	.on_bram	= on_bram,
	.on_data	= on_data,

	.on_commit	= on_commit,
};

#include <err.h>

int main (int argc, char *argv[])
{
	struct ctx o = { 0 };
	struct config c = { &action, &o };
	FILE *in;
	int ok;

	if (argc != 2)
		errx (0, "\n\ttrellis-test <conf-file>");

	if ((in = fopen (argv[1], "r")) == NULL)
		err (1, "cannot open conf file %s", argv[1]);

	ok = read_conf (&c, in);
	fclose (in);

	if (!ok)
		errx (1, c.error);

	return 0;
}
