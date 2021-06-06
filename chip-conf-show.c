/*
 * Config Parser Debug Helper
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chip-bits.h"
#include "chip-conf-show.h"

/* Global */

struct ctx {
	size_t i;
	int in_tile;
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

/* Tile Config */

static int on_tile (void *cookie, const char *name)
{
	struct ctx *o = cookie;

	printf ("tile %s\n", name);
	o->in_tile = 1;
	return 1;
}

static int on_raw (void *cookie, unsigned bit)
{
	struct ctx *o = cookie;

	if (o->in_tile)
		printf ("\t");

	printf ("raw =");
	chip_bit_write (bit, stdout);
	printf ("\n");
	return 1;
}

static int on_arrow (void *cookie, const char *sink, const char *source)
{
	struct ctx *o = cookie;

	if (o->in_tile)
		printf ("\t");

	printf ("arrow %s -> %s\n", source, sink);
	return 1;
}

static int on_mux (void *cookie, const char *name)
{
	printf ("mux %s\n", name);
	return 1;
}

static int on_mux_data (void *cookie, const char *source, unsigned *bits)
{
	printf ("\t%s =", source);
	chip_bits_write (bits, stdout);
	printf ("\n");
	return 1;
}

static int on_word (void *cookie, const char *name, const char *value)
{
	struct ctx *o = cookie;

	if (o->in_tile)
		printf ("\t");
	else
		o->i = strlen (value);

	printf ("word %s = %s\n", name, value);
	return 1;
}

static int on_word_data (void *cookie, unsigned *bits)
{
	struct ctx *o = cookie;

	printf ("\t[%zu] =", --o->i);
	chip_bits_write (bits, stdout);
	printf ("\n");
	return 1;
}

static int on_enum (void *cookie, const char *name, const char *value)
{
	struct ctx *o = cookie;

	if (o->in_tile)
		printf ("\t");

	printf ("enum %s = %s\n", name, value);
	return 1;
}

static int on_enum_data (void *cookie, const char *value, unsigned *bits)
{
	printf ("\t%s =", value);
	chip_bits_write (bits, stdout);
	printf ("\n");
	return 1;
}

/* BRAM */

static int on_bram (void *cookie, const char *name)
{
	struct ctx *o = cookie;

	printf ("bram %s", name);
	o->i = 0;
	return 1;
}

static int on_bram_data (void *cookie, unsigned value)
{
	struct ctx *o = cookie;
	const char *sep;

	sep = (o->i & 15) == 0 ? "\n\t" : (o->i & 15) == 8 ? "  " : " ";
	printf ("%s%03x", sep, value);
	++o->i;
	return 1;
}

/* Tile, Tile Group or BRAM */

static int on_commit (void *cookie)
{
	struct ctx *o = cookie;

	if (o->i > 0) {
		printf ("\n");
		o->i = 0;
	}

	printf ("\n");
	o->in_tile = 0;
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

struct chip_conf *chip_conf_show_alloc (void)
{
	struct ctx *o;
	struct chip_conf *c;

	if ((o = calloc (1, sizeof (*o))) == NULL)
		return NULL;

	if ((c = calloc (1, sizeof (*c))) == NULL)
		goto no_conf;

	c->action = &action;
	c->cookie = o;
	return c;
no_conf:
	free (o);
	return NULL;
}

void chip_conf_show_free (struct chip_conf *o)
{
	if (o == NULL)
		return;

	free (o->cookie);
	free (o);
}
