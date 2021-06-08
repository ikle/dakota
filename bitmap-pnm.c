/*
 * Dakota Chip Bitmap Export (to PBM)
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <string.h>

#include <dakota/bitmap.h>

static unsigned char reverse (unsigned char b)
{
	return ((b * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
}

struct bitmap *bitmap_import (const char *path)
{
	FILE *in;
	size_t width, height;
	struct bitmap *o;
	size_t i, size;
	int a;

	if ((in = fopen (path, "rb")) == NULL)
		return NULL;

	if (fscanf (in, "P4 %zu %zu ", &width, &height) != 2)
		goto no_header;

	if ((o = bitmap_alloc ()) == NULL)
		goto no_bitmap;

	if (!bitmap_resize (o, width - 1, height - 1))
		goto no_resize;

	for (size = o->pitch * o->height, i = 0; i < size; ++i) {
		if ((a = fgetc (in)) == EOF)
			goto no_data;

		o->mask[i] = 0xff;
		o->bits[i] = reverse (a);
	}

	fclose (in);
	return o;
no_data:
no_resize:
	bitmap_free (o);
no_bitmap:
no_header:
	fclose (in);
	return NULL;
}

int bitmap_export (const struct bitmap *o, const char *path)
{
	FILE *out;
	size_t i, size;

	if ((out = fopen (path, "wb")) == NULL)
		return 0;

	if (fprintf (out, "P4\n%zu %zu\n", o->width, o->height) < 0)
		goto error;

	for (size = o->pitch * o->height, i = 0; i < size; ++i)
		fputc (reverse (o->bits[i] & o->mask[i]), out);

	if (fclose (out) == 0)
		return 1;
error:
	fclose (out);
	remove (path);
	return 0;
}
