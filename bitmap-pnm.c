/*
 * Dakota Chip Bitmap Export (to PBM)
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include <dakota/bitmap.h>

static unsigned char reverse (unsigned char b)
{
	return ((b * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
}

static int pbm_import_data (FILE *in, unsigned char *data, size_t count)
{
	size_t i;
	int a;

	for (i = 0; i < count; ++i) {
		if ((a = fgetc (in)) == EOF)
			return 0;

		data[i] = reverse (a);
	}

	return 1;
}

static unsigned char *pbm_import (FILE *in, size_t *w, size_t *h)
{
	unsigned char *data;
	size_t count;

	if (fscanf (in, "P4 %zu %zu ", w, h) != 2)
		return NULL;

	count = *w * *h;

	if ((data = malloc (count)) == NULL)
		return NULL;

	if (pbm_import_data (in, data, count))
		return data;

	free (data);
	return 0;
}

static int pbm_export_data (FILE *out, const unsigned char *data, size_t count)
{
	size_t i;

	for (i = 0; i < count; ++i)
		if (fputc (reverse (data[i]), out) == EOF)
			return 0;

	return 1;
}

static int pbm_export (FILE *out, size_t w, size_t h, const unsigned char *data)
{
	size_t count = w * h;

	if (fprintf (out, "P4\n%zu %zu\n", w, h) < 0)
		return 0;

	return pbm_export_data (out, data, count);
}

struct bitmap *bitmap_import (const char *path)
{
	FILE *in;
	size_t bw, bh, mw, mh;
	unsigned char *bits, *mask;
	struct bitmap *o;

	if ((in = fopen (path, "rb")) == NULL)
		return NULL;

	bits = pbm_import (in, &bw, &bh);
	mask = pbm_import (in, &mw, &mh);

	fclose (in);

	if (bits == NULL || mask == NULL || bw != mw || bh != mh)
		goto no_import;

	if ((o = bitmap_alloc ()) == NULL)
		goto no_bitmap;

	o->width  = bw;
	o->height = bh;
	o->bits   = bits;
	o->mask   = mask;
	return o;
no_bitmap:
no_import:
	free (bits);
	free (mask);
	return NULL;
}

int bitmap_export (const struct bitmap *o, const char *path)
{
	FILE *out;

	if ((out = fopen (path, "wb")) == NULL)
		return 0;

	if (!pbm_export (out, o->width, o->height, o->bits) ||
	    !pbm_export (out, o->width, o->height, o->mask))
		goto error;

	if (fclose (out) == 0)
		return 1;
error:
	fclose (out);
	remove (path);
	return 0;
}
