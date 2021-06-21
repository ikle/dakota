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

struct ctx {
	FILE *out;
	int indent;
};

static int show_indent (struct ctx *c)
{
	int n, ok = 1;

	for (n = c->indent; n > 0; --n)
		ok &= fputc ('\t', c->out) != EOF;

	return ok;
}

static int writer (void *cookie, int type, int x, int y, ...)
{
	struct ctx *c = cookie;
	va_list ap;
	int n, angle, dir;
	const char *mark, *text, *name;

	if (type == SYMBOL_END)
		--c->indent;

	if (!show_indent (c))
		return 0;

	va_start (ap, y);

	switch (type) {
	case SYMBOL_MOVE:
		n = fprintf (c->out, "move %d %d\n", x, y);
		break;
	case SYMBOL_LINE:
		n = fprintf (c->out, "line %d %d\n", x, y);
		break;
	case SYMBOL_ARC:
		angle = va_arg (ap, int);
		n = fprintf (c->out, "arc %d %d %d\n", x, y, angle);
		break;
	case SYMBOL_MARK:
		mark = va_arg (ap, const char *);
		n = fprintf (c->out, "mark %d %d %s\n", x, y, mark);
		break;
	case SYMBOL_TEXT:
		dir = va_arg (ap, int);
		text = va_arg (ap, const char *);
		n = fprintf (c->out, "text %d %d %c %s\n", x, y, dir, text);
		break;
	case SYMBOL_BLIT:
		dir = va_arg (ap, int);
		name = va_arg (ap, const char *);
		n = fprintf (c->out, "blit %d %d %d %s\n", x, y, dir, name);
		break;
	case SYMBOL_TILE:
		name = va_arg (ap, const char *);
		n = fprintf (c->out, "tile %s\n", name);
		++c->indent;
		break;
	case SYMBOL_END:
		n = fprintf (c->out, "end\n");
		break;
	default:
		n = 0;
	}

	va_end (ap);
	return n >= 0;
}

int symbol_write (const struct symbol *o, const char *path)
{
	struct ctx c = {NULL, 0};

	if ((c.out = fopen (path, "w")) == NULL)
		return 0;

	if (!symbol_walk (o, writer, &c))
		goto no_walk;

	return fclose (c.out) == 0;
no_walk:
	fclose (c.out);
	return 0;
}
