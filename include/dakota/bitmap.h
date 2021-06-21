/*
 * Dakota Chip Bitmap
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_BITMAP_H
#define DAKOTA_BITMAP_H  1

#include <stddef.h>

#include <dakota/chip-bits.h>

struct bitmap {
	size_t width, height;	/* in bits  */
	size_t pitch;		/* in bytes */

	unsigned char *bits;
	unsigned char *mask;
};

struct bitmap *bitmap_alloc (void);
struct bitmap *bitmap_clone (const struct bitmap *o);
void bitmap_free (struct bitmap *o);

int bitmap_resize (struct bitmap *o, size_t x, size_t y);

int  bitmap_add_bits (struct bitmap *o, const unsigned *bits);
void bitmap_sub_bits (struct bitmap *o, const unsigned *bits);

int bitmap_blit (struct bitmap *o, size_t x, size_t y,
		 const struct bitmap *tile);

struct bitmap *bitmap_import (const char *path);
int bitmap_export (const struct bitmap *o, const char *path);

#endif  /* DAKOTA_BITMAP_H */
