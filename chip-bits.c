/*
 * Chip Bit Helpers
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <errno.h>
#include <stdlib.h>

#include "chip-bits.h"

int chip_bit_read (FILE *in)
{
	char la;
	int value;
	unsigned x, y;

	if (fscanf (in, "%*[ \t]%c", &la) != 1)
		goto error;

	if (la == '-')
		goto empty;

	value = (la != '!') ? 1 : 0;

	if (la != '!')
		ungetc (la, in);

	if (la == '\n')
		goto error;

	if (fscanf (in, "F%uB%u", &x, &y) != 2)
		goto error;

	if (x > 127 || y > 127)
		goto error;

	return chip_bit_make (x, y, value);
empty:
	errno = 0;
	return -1;
error:
	errno = EILSEQ;
	return -1;
}

int chip_bit_write (unsigned bit, FILE *out)
{
	const char *prefix = (bit & 0x80) == 0 ? "!" : "";
	unsigned x = (bit >> 8) & 0x7f;
	unsigned y = bit & 0x7f;

	return fprintf (out, " %sF%uB%u", prefix, x, y) > 0;
}

unsigned *chip_bits_read (FILE *in)
{
	int bit;
	size_t i, max = 1;
	unsigned *bits, *p;

	if ((bit = chip_bit_read (in)) < 0)
		return NULL;

	if ((bits = malloc (sizeof (bits[0]) * max)) == NULL)
		return NULL;

	bits[0] = bit;

	for (i = 0; (bit = chip_bit_read (in)) >= 0; bits[i] = bit) {
		bits[i++] |= 0x8000;

		if (i >= max) {
			max *= 2;
			p = realloc (bits, sizeof (bits[0]) * max);
			if (p == NULL)
				goto no_mem;

			bits = p;
		}
	}

	return bits;
no_mem:
	free (bits);
	return NULL;
}

int chip_bits_write (unsigned *bits, FILE *out)
{
	if (bits == NULL) {
		fprintf (out, " -");
		return 1;
	}

	do {
		if (!chip_bit_write (bits[0], out))
			return 0;
	}
	while (!chip_bit_last (*bits++));

	return 1;
}
