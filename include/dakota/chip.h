/*
 * Dakota Chip
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_CHIP_H
#define DAKOTA_CHIP_H  1

#include <cmdb.h>
#include <dakota/bitmap.h>

struct chip *chip_alloc (struct cmdb *tiles, struct cmdb *grid);
void chip_free (struct chip *o);

int chip_add_grid (struct chip *o, struct cmdb *grid);
int chip_add_tile (struct chip *o, const char *name, const char *type);

int chip_set_raw  (struct chip *o, const unsigned *bits);
int chip_set_mux  (struct chip *o, const char *name, const char *source);
int chip_set_word (struct chip *o, const char *name, const char *value);
int chip_set_enum (struct chip *o, const char *name, const char *value);

int chip_commit (struct chip *o);

const struct bitmap *chip_get_bits (const struct chip *o);

#endif  /* DAKOTA_CHIP_H */
