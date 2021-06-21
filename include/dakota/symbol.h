/*
 * Dakota Symbol
 *
 * Note, it is NOT generic graphics library.
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_SYMBOL_H
#define DAKOTA_SYMBOL_H  1

#include <stddef.h>

struct symbol *symbol_alloc (struct symbol *parent, const char *name);
void symbol_free (struct symbol *o);

int symbol_move (struct symbol *o, int x, int y);
int symbol_line (struct symbol *o, int x, int y);
int symbol_arc  (struct symbol *o, int x, int y, int angle);
int symbol_mark (struct symbol *o, int x, int y, const char *mark);
int symbol_text (struct symbol *o, int x, int y, int dir, const char *text);
int symbol_blit (struct symbol *o, int x, int y, const struct symbol *tile);

int symbol_add_tile (struct symbol *o, struct symbol *tile);

struct symbol *symbol_read (const char *name, const char *path);
int symbol_write (const struct symbol *o, const char *path);

enum symbol_type {
	SYMBOL_MOVE,
	SYMBOL_LINE,
	SYMBOL_ARC,
	SYMBOL_MARK,
	SYMBOL_TEXT,

	SYMBOL_TILE,
	SYMBOL_END,
};

typedef int symbol_fn (void *cookie, int type, int x, int y, ...);

int symbol_walk (const struct symbol *o, symbol_fn *fn, void *cookie);

#endif  /* DAKOTA_SYMBOL_H */
