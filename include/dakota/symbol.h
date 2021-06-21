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

struct symbol *symbol_alloc (void);
void symbol_free (struct symbol *o);

int symbol_move (struct symbol *o, int x, int y);
int symbol_line (struct symbol *o, int x, int y);
int symbol_arc  (struct symbol *o, int x, int y, int degree);
int symbol_mark (struct symbol *o, int x, int y, const char *mark);
int symbol_text (struct symbol *o, int x, int y, const char *text);
int symbol_blit (struct symbol *o, int x, int y, const struct symbol *tile);

struct symbol *symbol_read (const char *path);
int symbol_write (const struct symbol *o, const char *path);

#endif  /* DAKOTA_SYMBOL_H */
