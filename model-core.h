/*
 * Dakota Model Core
 *
 * Copyright (c) 2021-2022 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_MODEL_CORE_H
#define DAKOTA_MODEL_CORE_H  1

#include <stddef.h>

#include <dakota/data/pair.h>
#include <dakota/error.h>
#include <dakota/model/cell.h>
#include <dakota/model/port.h>

struct model {
	struct model *parent;
	char *name;
	struct model *last;

	size_t        nparams;
	struct pair  *param;
	size_t        nports;
	struct port  *port;
	size_t        ncells;
	struct cell  *cell;
	size_t        nmodels;
	struct model *model;	/* referenced and/or secondary models */

	struct error  error;
};

int  model_init (struct model *o, struct model *parent, const char *name);
void model_fini (struct model *o);

int model_add_port (struct model *o, const char *name, int type,
		    struct cell *cell, size_t ref);

#define M_UNKNOWN  ((size_t) -1)

size_t model_get_port (struct model *o, const char *name);
struct model *model_get_model (struct model *o, const char *name);

#endif  /* DAKOTA_MODEL_H */
