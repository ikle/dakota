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

#include "model-pair.h"
#include "model-tuple.h"

struct cell {
	char *type;
	char *name;

	size_t ni, no;		/* hint for number of inputs and outputs */

	size_t        ntuples;
	struct tuple *tuple;
	size_t        nparams;
	struct pair  *param;
	size_t        nattrs;
	struct pair  *attr;
};

int  cell_init (struct cell *o, const char *type, const char *name);
void cell_fini (struct cell *o);

int cell_add_tuple_va (struct cell *o, int size, va_list ap);
int cell_add_tuple_v  (struct cell *o, int size, const char *argv[]);
int cell_add_tuple    (struct cell *o, int size, ...);
int cell_add_param    (struct cell *o, const char *name, const char *value);
int cell_add_attr     (struct cell *o, const char *name, const char *value);

const char *cell_get_attr (const struct cell *o, const char *name);

#endif  /* DAKOTA_MODEL_CELL_H */
