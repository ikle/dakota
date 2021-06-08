/*
 * Dakota Chip Bitmap
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include <dakota/bitmap.h>

struct bitmap *bitmap_alloc (void)
{
	struct bitmap *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	o->width  = 0;
	o->height = 0;
	o->pitch  = 0;
	o->bits   = NULL;
	o->mask   = NULL;
	return o;
}

void bitmap_free (struct bitmap *o)
{
	if (o == NULL)
		return;

	free (o->bits);
	free (o->mask);
	free (o);
}

struct bitmap *bitmap_clone (const struct bitmap *from)
{
	struct bitmap *o;
	size_t size;

	if ((o = bitmap_alloc ()) == NULL)
		return NULL;

	size = from->pitch * from->height;

	if ((o->bits = malloc (size)) == NULL ||
	    (o->mask = malloc (size)) == NULL)
		goto no_mem;

	memcpy (o->bits, from->bits, size);
	memcpy (o->mask, from->mask, size);

	o->width  = from->width;
	o->height = from->height;
	o->pitch  = from->pitch;
	return o;
no_mem:
	bitmap_free (o);
	return NULL;
}

#define GET_PITCH(x)	(((x) + 7) >> 3)

int bitmap_resize (struct bitmap *o, size_t x, size_t y)
{
	size_t width  = (x < o->width)  ? o->width  : x + 1;
	size_t height = (y < o->height) ? o->height : y + 1;
	size_t pitch  = GET_PITCH (width);
	size_t size;
	unsigned char *bits, *mask;

	if (pitch == o->pitch && height == o->height) {
		o->width = width;
		return 1;
	}

	size = pitch * height;

	if ((bits = calloc (size, 1)) == NULL)
		return 0;

	if ((mask = calloc (size, 1)) == NULL)
		goto no_mask;

	for (y = 0; y < o->height; ++y) {
		memcpy (bits + y * pitch, o->bits + y * o->pitch, o->pitch);
		memcpy (mask + y * pitch, o->mask + y * o->pitch, o->pitch);
	}

	free (o->bits);
	free (o->mask);

	o->width  = width;
	o->height = height;
	o->pitch  = pitch;
	o->bits   = bits;
	o->mask   = mask;
	return 1;
no_mask:
	free (bits);
	return 0;
}

static int bitmap_put (struct bitmap *o, size_t x, size_t y, int value)
{
	size_t i;
	unsigned char pattern;

	if (!bitmap_resize (o, x, y))
		return 0;

	i = y * o->pitch + (x >> 3);
	pattern = 1 << (x & 7);

	if (value)
		o->bits[i] |= pattern;
	else
		o->bits[i] &= ~pattern;

	o->mask[i] |= pattern;
	return 1;
}

int bitmap_add (struct bitmap *o, const unsigned *bits)
{
	int x, y;

	if (bits == NULL)
		return 1;

	do {
		x = chip_bit_x (*bits);
		y = chip_bit_y (*bits);

		if (!bitmap_put (o, x, y, chip_bit_value (*bits)))
			return 0;
	}
	while (!chip_bit_last (*bits++));

	return 1;
}

static void bitmap_drop (struct bitmap *o, size_t x, size_t y)
{
	size_t i;
	unsigned char pattern;

	if (x >= o->width || y >= o->height)
		return;

	i = y * o->pitch + (x >> 3);
	pattern = 1 << (x & 7);

	o->mask[i] &= ~pattern;
}

void bitmap_sub (struct bitmap *o, const unsigned *bits)
{
	int x, y;

	if (bits == NULL)
		return;

	do {
		x = chip_bit_x (*bits);
		y = chip_bit_y (*bits);

		bitmap_drop (o, x, y);
	}
	while (!chip_bit_last (*bits++));
}
