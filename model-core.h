/*
 * Dakota Model Core
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_MODEL_CORE_H
#define DAKOTA_MODEL_CORE_H  1

#include <stddef.h>

#include <dakota/error.h>

#include "model-port.h"
#include "model-wire.h"
#include "model-cell.h"

struct model {
	char *name;
	struct model *last;

	size_t        nports;
	struct port  *port;
	size_t        nwires;
	struct wire  *wire;
	size_t        ncells;
	struct cell  *cell;
	size_t        nmodels;
	struct model *model;	/* referenced and/or secondary models */

	struct error  error;
};

int  model_init (struct model *o, const char *name);
void model_fini (struct model *o);

int
model_add_port (struct model *o, struct cell *cell, const char *name, int type);

struct port  *model_get_port  (struct model *o, const char *name);
struct model *model_get_model (struct model *o, const char *name);

#endif  /* DAKOTA_MODEL_H */
