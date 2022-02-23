/*
 * Dakota Model Cell
 *
 * Copyright (c) 2021-2022 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_MODEL_CELL_H
#define DAKOTA_MODEL_CELL_H  1

#include <stdarg.h>
#include <stddef.h>

#include <dakota/bitmap.h>
#include <dakota/data/pair.h>
#include <dakota/model/tuple.h>

struct cell {
	char *type;
	char *name;

	size_t        nbinds;
	struct pair  *bind;
	size_t        ntuples;
	struct tuple *tuple;
	size_t        nparams;
	struct pair  *param;
	size_t        nattrs;
	struct pair  *attr;

	struct bitmap *map;
};

int  cell_init (struct cell *o, const char *type, const char *name);
void cell_fini (struct cell *o);

int cell_add_bind     (struct cell *o, const char *port, const char *value);

int cell_add_tuple_va (struct cell *o, int size, va_list ap);
int cell_add_tuple_v  (struct cell *o, int size, const char *argv[]);
int cell_add_tuple    (struct cell *o, int size, ...);
int cell_add_param    (struct cell *o, const char *name, const char *value);
int cell_add_attr     (struct cell *o, const char *name, const char *value);

int cell_load_bitmap  (struct cell *o, const char *path);

const char *cell_get_attr (const struct cell *o, const char *name);

#endif  /* DAKOTA_MODEL_CELL_H */
