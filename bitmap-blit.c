/*
 * Dakota Chip Bitmap Blit
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <dakota/bitmap.h>

int bitmap_blit (struct bitmap *o, size_t x, size_t y,
		 const struct bitmap *tile)
{
	const size_t rshift = (x & 7);
	const size_t lshift = 8 - rshift;

	unsigned prev_mask, prev_bits, mask, bits;
	size_t start_src, start_dst, src, dst, i, j;

	if (tile->width == 0 || tile->height == 0)
		return 1;

	if (!bitmap_resize (o, x + tile->width - 1, y + tile->height - 1))
		return 0;

	for (
		start_src = 0, start_dst = y * o->pitch + (x >> 3), j = 0;
		j < tile->height;
		start_src += tile->pitch, start_dst += o->pitch, ++j
	)
		for (
			prev_mask = 0, prev_bits = 0,
			src = start_src, dst = start_dst, i = 0;
			i < tile->pitch;
			++src, ++dst, ++i
		) {
			mask = (tile->mask[src] << rshift) | prev_mask;
			bits = (tile->bits[src] << rshift) | prev_bits;

			o->mask[dst] |=  mask;
			o->bits[dst] &= ~mask;
			o->bits[dst] |=  bits;

			prev_mask = (tile->mask[src] >> lshift);
			prev_bits = (tile->bits[src] >> lshift);
		}

	return 1;
}
