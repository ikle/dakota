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
	const size_t shift = (x & 7);

	unsigned prev_mask = 0, prev_bits = 0, mask, bits;
	size_t start_src, start_dst, src, dst, i;

	if (tile->width == 0 || tile->height == 0)
		return 1;

	if (!bitmap_resize (o, x + tile->width - 1, y + tile->height - 1))
		return 0;

	for (
		start_src = 0, start_dst = y * o->pitch + (x >> 3);
		y < tile->height;
		start_src += tile->pitch, start_dst += o->pitch, ++y
	)
		for (
			src = start_src, dst = start_dst, i = 0;
			i < tile->pitch;
			++src, ++dst, ++i
		) {
			mask = (tile->mask[src] << shift) | prev_mask;
			bits = (tile->bits[src] << shift) | prev_bits;

			o->mask[dst] |=  mask;
			o->bits[dst] &= ~mask;
			o->bits[dst] |=  bits;

			prev_mask = (tile->mask[src] >> shift);
			prev_bits = (tile->bits[src] >> shift);
		}

	return 1;
}