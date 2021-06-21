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

int symbol_add_move (struct symbol *o, int x, int y);
int symbol_add_line (struct symbol *o, int x, int y);
int symbol_add_arc  (struct symbol *o, int x, int y, int angle);
int symbol_add_mark (struct symbol *o, int x, int y, const char *mark);
int symbol_add_text (struct symbol *o, int x, int y, int dir, const char *text);
int symbol_add_blit (struct symbol *o, int x, int y, int dir, const char *name);

int symbol_add_tile (struct symbol *o, struct symbol *tile);
struct symbol *symbol_get_tile (struct symbol *o, const char *name);

struct symbol *symbol_read (const char *name, const char *path);
int symbol_write (const struct symbol *o, const char *path);

enum symbol_type {
	SYMBOL_MOVE	= 0,
	SYMBOL_LINE	= 1,
	SYMBOL_ARC	= 2,
	SYMBOL_MARK	= 3,
	SYMBOL_TEXT	= 4,
	SYMBOL_BLIT	= 5,

	SYMBOL_TILE	= 14,
	SYMBOL_END	= 15,
};

typedef int symbol_fn (void *cookie, int type, int x, int y, ...);

int symbol_walk (const struct symbol *o, symbol_fn *fn, void *cookie);

#endif  /* DAKOTA_SYMBOL_H */
