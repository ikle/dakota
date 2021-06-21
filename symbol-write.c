/*
 * Dakota Symbol Writer
 *
 * Note, it is NOT generic graphics library.
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdarg.h>
#include <stdio.h>

#include <dakota/symbol.h>

static int writer (void *cookie, int type, int x, int y, ...)
{
	FILE *out = cookie;
	va_list ap;
	int n, angle, dir;
	const char *mark, *text;

	va_start (ap, y);

	switch (type) {
	case SYMBOL_MOVE:
		n = fprintf (out, "move %d %d\n", x, y);
		break;
	case SYMBOL_LINE:
		n = fprintf (out, "line %d %d\n", x, y);
		break;
	case SYMBOL_ARC:
		angle = va_arg (ap, int);
		n = fprintf (out, "arc %d %d %d\n", x, y, angle);
		break;
	case SYMBOL_MARK:
		mark = va_arg (ap, const char *);
		n = fprintf (out, "mark %d %d %s\n", x, y, mark);
		break;
	case SYMBOL_TEXT:
		dir = va_arg (ap, int);
		text = va_arg (ap, const char *);
		n = fprintf (out, "text %d %d %c %s\n", x, y, dir, text);
		break;
	default:
		n = 0;
	}

	va_end (ap);
	return n >= 0;
}

int symbol_write (const struct symbol *o, const char *path)
{
	FILE *out;

	if ((out = fopen (path, "w")) == NULL)
		return 0;

	if (!symbol_walk (o, writer, out))
		goto no_walk;

	return fclose (out) == 0;
no_walk:
	fclose (out);
	return 0;
}