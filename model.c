/*
 * Dakota Model
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include <dakota/error.h>
#include <dakota/model.h>
#include <dakota/string.h>

#include "model-core.h"

struct model *model_alloc (const char *name)
{
	struct model *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	if (!model_init (o, name))
		goto no_init;

	return o;
no_init:
	free (o);
	return NULL;
}

void model_free (struct model *o)
{
	if (o == NULL)
		return;

	model_fini (o);
	free (o);
}

/*
 * 1. find cell model, and
 * 2. create local ports binded to cell ports
 */
static int model_bind_cell (struct model *o, struct cell *cell)
{
	struct model *m;
	size_t i;
	char *name;
	int ok = 1, type;

	if ((m = model_get_model (o, cell->type)) == NULL)
		return error (&o->error, "cannot find model %s for cell %s",
			      cell->type, cell->name);

	o->last = m;

	for (i = 0; i < m->nports; ++i) {
		name = make_string ("%s.%s", cell->name, m->port[i].name);
		type = m->port[i].type | PORT_LOCAL;

		ok &= name != NULL && model_add_port (o, cell, name, type);
		free (name);
	}

	return ok ? 1 : error (&o->error, NULL);
}

/*
 * 1. find model ports to connect, and
 * 2. validate types
 */
static int model_bind_wire (struct model *o, struct wire *wire)
{
	const struct port *sink, *source;

	if ((sink = model_get_port (o, wire->sink)) == NULL)
		return error (&o->error, "cannot find sink port %s",
			      wire->sink);

	if ((sink->type & PORT_TYPE) != PORT_SINK)
		return error (&o->error, "cannot use source port %s as sink",
			      wire->sink);

	if ((source = model_get_port (o, wire->source)) == NULL)
		return error (&o->error, "cannot find source port %s",
			      wire->source);

	if ((source->type & PORT_TYPE) != PORT_SOURCE)
		return error (&o->error, "cannot use sink port %s as source",
			      wire->source);

	return 1;
}

int model_commit (struct model *o)
{
	size_t i;

	for (i = 0; i < o->ncells; ++i)
		if (!model_bind_cell (o, o->cell + i))
			return 0;

	for (i = 0; i < o->nwires; ++i)
		if (!model_bind_wire (o, o->wire + i))
			return 0;

	return 1;
}
