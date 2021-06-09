/*
 * Dakota Chip Tile Group
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_CHIPLET_H
#define DAKOTA_CHIPLET_H  1

#include <cmdb.h>
#include <dakota/bitmap.h>

struct chiplet *chiplet_alloc (struct cmdb *db);
void chiplet_reset (struct chiplet *o);
void chiplet_free  (struct chiplet *o);

int chiplet_add (struct chiplet *o, size_t x, size_t y, const char *type);

int chiplet_set_raw  (struct chiplet *o, const unsigned *bits);
int chiplet_set_mux  (struct chiplet *o, const char *name, const char *source);
int chiplet_set_word (struct chiplet *o, const char *name, const char *value);
int chiplet_set_enum (struct chiplet *o, const char *name, const char *value);

int chiplet_blit (const struct chiplet *o, struct bitmap *image);

#endif  /* DAKOTA_CHIPLET_H */
