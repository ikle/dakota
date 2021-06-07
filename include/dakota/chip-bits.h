/*
 * Dakota Chip Bit Helpers
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_CHIP_BITS_H
#define DAKOTA_CHIP_BITS_H  1

#include <stdio.h>

#ifndef assert
#define assert(e)
#endif

static inline unsigned chip_bit_make (int x, int y, int value)
{
	assert ((x & ~0x7f)  == 0);
	assert ((y & ~0x7f)  == 0);
	assert ((value & ~1) == 0);

	return (x << 8) | (value << 7) | y;
}

static inline int chip_bit_last (unsigned bit)
{
	return (bit & 0x8000) == 0;
}

static inline int chip_bit_x (unsigned bit)
{
	return ((bit >> 8) & 0x7f);
}

static inline int chip_bit_value (unsigned bit)
{
	return (bit & 0x80) != 0;
}

static inline int chip_bit_y (unsigned bit)
{
	return (bit & 0x7f);
}

int chip_bit_parse (const char *s);
unsigned *chip_bits_parse (const char *s);

int chip_bit_read (FILE *in);
int chip_bit_write (unsigned bit, FILE *out);

unsigned *chip_bits_read (FILE *in);
int chip_bits_write (const unsigned *bits, FILE *out);

unsigned *chip_bits_merge (unsigned *what, const unsigned *with);

#endif  /* DAKOTA_CHIP_BITS_H */
