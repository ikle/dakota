/*
 * Dakota Chip Tile
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_TILE_H
#define DAKOTA_TILE_H  1

#include <cmdb.h>
#include <dakota/bitmap.h>

struct tile *tile_alloc (struct cmdb *db, const char *type);
void tile_free (struct tile *o);

int tile_set_raw  (struct tile *o, const unsigned *bits);
int tile_set_mux  (struct tile *o, const char *name, const char *source);
int tile_set_word (struct tile *o, const char *name, const char *value);
int tile_set_enum (struct tile *o, const char *name, const char *value);

const struct bitmap *tile_get_bits (const struct tile *o);

#endif  /* DAKOTA_TILE_H */
