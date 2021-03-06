/*
 * Chip Bit Helpers
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <dakota/chip-bits.h>

int chip_bit_parse (const char *s)
{
	int value = 1;
	unsigned x, y;

	s += strspn (s, " \t");

	if (*s == '-') {
		errno = 0;
		return -1;
	}

	if (*s == '!')
		++s, value = 0;

	if (sscanf (s, "F%uB%u", &x, &y) != 2)
		goto error;

	if (x > 127 || y > 127)
		goto error;

	return chip_bit_make (x, y, value);
error:
	errno = EILSEQ;
	return -1;
}

static const char *next_word (const char *s)
{
	s += strcspn (s, " \t");
	s += strspn  (s, " \t");

	return s;
}

static size_t chip_bits_count (const char *s)
{
	size_t count;

	s += strspn (s, " \t");

	for (count = 0; *s != '\0'; ++count)
		s = next_word (s);

	return count;
}

unsigned *chip_bits_parse (const char *s)
{
	int bit;
	size_t count, i;
	unsigned *bits;

	if ((bit = chip_bit_parse (s)) < 0)
		return NULL;

	count = chip_bits_count (s);

	if ((bits = malloc (sizeof (bits[0]) * count)) == NULL)
		return NULL;

	bits[0] = bit;
	s = next_word (s);

	for (i = 0; (bit = chip_bit_parse (s)) >= 0; bits[i] = bit) {
		bits[i++] |= 0x8000;
		s = next_word (s);
	}

	return bits;
}

void chip_bits_invert (unsigned *bits)
{
	if (bits == NULL)
		return;

	do {
		*bits = chip_bit_invert (*bits);
	}
	while (!chip_bit_last (*bits++));
}

int chip_bit_read (FILE *in)
{
	int la;
	char word[16];

	while ((la = getc (in)) != EOF)
		if (la != ' ' && la != '\t') {
			ungetc (la, in);
			break;
		}

	if (la == '\n')
		goto error;

	if (fscanf (in, "%15s", word) != 1)
		goto error;

	return chip_bit_parse (word);
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

static int chip_bit_print (char *to, size_t size, unsigned bit)
{
	const char *prefix = (bit & 0x80) == 0 ? "!" : "";
	unsigned x = (bit >> 8) & 0x7f;
	unsigned y = bit & 0x7f;

	return snprintf (to, size, "%sF%uB%u", prefix, x, y);
}

static int chip_bits_print (char *to, size_t size, const unsigned *bits)
{
	int len, total = 0;

	if (bits == NULL)
		return snprintf (to, size, "-");

	for (;;) {
		len = chip_bit_print (to, size, bits[0]);

		total += len, to += len, size = size > len ? size - len : 0;

		if (chip_bit_last (*bits++))
			break;

		total += snprintf (to, size, " ");
		++to, size = size > 1 ? size - 1 : 0;
	}

	return total;
}

char *chip_bits_string (const unsigned *bits)
{
	int size = chip_bits_print (NULL, 0, bits) + 1;
	char *s;

	if ((s = malloc (size)) == NULL)
		return NULL;

	chip_bits_print (s, size, bits);
	return s;
}

int chip_bits_write (const unsigned *bits, FILE *out)
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

static size_t chip_bits_len (const unsigned *bits)
{
	size_t len;

	if (bits == NULL)
		return 0;

	for (len = 1; !chip_bit_last (*bits); ++bits, ++len) {}

	return len;
}

unsigned *chip_bits_merge (unsigned *what, const unsigned *with)
{
	size_t what_len = chip_bits_len (what);
	size_t with_len = chip_bits_len (with);
	size_t total = what_len + with_len;
	unsigned *bits;

	if ((bits = realloc (what, sizeof (bits[0]) * total)) == NULL)
		return NULL;

	if (what_len > 0 && with_len > 0)
		bits[what_len - 1] |= 0x8000;

	memcpy (bits + what_len, with, sizeof (bits[0]) * with_len);
	return bits;
}
