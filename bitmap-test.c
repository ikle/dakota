/*
 * Dakota Bitmap Test
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <err.h>
#include <stdlib.h>

#include <dakota/bitmap.h>

static char sample[] = "F0B6 F1B7 F2B7 F3B7 F4B0 F4B1 F4B2 F4B3 F4B4 F4B5 F4B6";

int main (int argc, char *argv[])
{
	struct bitmap *image, *tile;
	unsigned *bits;
	int ok;

	if ((image = bitmap_alloc ()) == NULL)
		errx (1, "cannot allocate image bitmap");

	if ((tile = bitmap_alloc ()) == NULL)
		errx (1, "cannot allocate tile bitmap");

	bits = chip_bits_parse (sample);

	if (!bitmap_add (tile, bits))
		err (1, "cammot add bits to tile");

	ok = bitmap_blit (image,  0,  0, tile) &&
	     bitmap_blit (image,  3,  3, tile) &&
	     bitmap_blit (image,  4,  6, tile) &&
	     bitmap_blit (image,  5,  9, tile) &&
	     bitmap_blit (image, 15,  5, tile) &&
	     bitmap_blit (image, 20, 17, tile);

	if (!ok)
		err (1, "cannot blit tile to image");

	if (!bitmap_export (tile, "test/bitmap-tile.pnm"))
		err (1, "cannot export tile");

	if (!bitmap_export (image, "test/bitmap-image.pnm"))
		err (1, "cannot export image");

	free (bits);
	bitmap_free (tile);
	bitmap_free (image);
	return 0;
}
