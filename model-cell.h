/*
 * Dakota Cell
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_MODEL_CELL_H
#define DAKOTA_MODEL_CELL_H  1

#include <stdarg.h>
#include <stddef.h>

struct pair {
	char *key, *value;
};

int  pair_init (struct pair *o, const char *key, const char *value);
void pair_fini (struct pair *o);

struct tuple {
	int size;
	char **m;
};

int  tuple_init (struct tuple *o, int size, va_list ap);
void tuple_fini (struct tuple *o);

struct cell {
	char *type;
	char *name;

	size_t        ntuples;
	struct tuple *tuple;
	size_t        nparams;
	struct pair  *param;
	size_t        nattrs;
	struct pair  *attr;
};

int  cell_init (struct cell *o, const char *type, const char *name);
void cell_fini (struct cell *o);

int cell_add_tuple (struct cell *o, int size, ...);
int cell_add_param (struct cell *o, const char *name, const char *value);
int cell_add_attr  (struct cell *o, const char *name, const char *value);

#endif  /* DAKOTA_MODEL_CELL_H */
