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

static unsigned char sample_msb (unsigned char b)
{
	return ((b * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
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
		fputc (sample_msb (o->bits[i] & o->mask[i]), out);

	if (fclose (out) == 0)
		return 1;
error:
	fclose (out);
	remove (path);
	return 0;
}
