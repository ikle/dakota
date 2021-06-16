/*
 * Dakota Model Core
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include <dakota/array.h>
#include <dakota/error.h>
#include <dakota/model.h>

#include "model-core.h"

int model_init (struct model *o, const char *name)
{
	if ((o->name = strdup (name)) == NULL)
		return 0;

	o->last = o;

	o->nports  = 0;
	o->port    = NULL;
	o->nwires  = 0;
	o->wire    = NULL;
	o->ncells  = 0;
	o->cell    = NULL;
	o->nmodels = 0;
	o->model   = NULL;

	error_init (&o->error);
	return 1;
}

void model_fini (struct model *o)
{
	free (o->name);

	array_free (o->port,  o->nports,  port_fini);
	array_free (o->wire,  o->nwires,  wire_fini);
	array_free (o->cell,  o->ncells,  cell_fini);
	array_free (o->model, o->nmodels, model_fini);

	error_fini (&o->error);
}

const char *model_status (struct model *o)
{
	return o->error.reason;
}

int
model_add_port (struct model *o, struct cell *cell, const char *name, int type)
{
	struct model *m = o->last;
	const size_t nports = m->nports + 1;
	struct port *p;

	if ((p = array_resize (m->port, nports)) == NULL)
		return error (&o->error, NULL);

	m->port = p;

	if (!port_init (m->port + m->nports, cell, name, type))
		return error (&o->error, NULL);

	m->nports = nports;
	return 1;
}

int model_add_input (struct model *o, const char *name)
{
	return model_add_port (o, NULL, name, PORT_INPUT | PORT_DRIVEN);
}

int model_add_output (struct model *o, const char *name)
{
	return model_add_port (o, NULL, name, 0);
}

int model_add_wire (struct model *o, const char *sink, const char *source)
{
	struct model *m = o->last;
	const size_t nwires = m->nwires + 1;
	struct wire *p;

	if ((p = array_resize (m->wire, nwires)) == NULL)
		return error (&o->error, NULL);

	m->wire = p;

	if (!wire_init (m->wire + m->nwires, sink, source))
		return error (&o->error, NULL);

	m->nwires = nwires;
	return 1;
}

int model_add_cell (struct model *o, const char *type, const char *name)
{
	struct model *m = o->last;
	const size_t ncells = m->ncells + 1;
	struct cell *p;

	if ((p = array_resize (m->cell, ncells)) == NULL)
		return error (&o->error, NULL);

	m->cell = p;

	if (!cell_init (m->cell + m->ncells, type, name))
		return error (&o->error, NULL);

	m->ncells = ncells;
	return 1;
}

int model_add_model (struct model *o, const char *name)
{
	const size_t nmodels = o->nmodels + 1;
	struct model *p;

	if ((p = array_resize (o->model, nmodels)) == NULL)
		return error (&o->error, NULL);

	o->model = p;

	if (!model_init (o->model + o->nmodels, name))
		return error (&o->error, NULL);

	o->last = o->model + o->nmodels;

	o->nmodels = nmodels;
	return 1;
}

int model_add_tuple (struct model *o, int size, ...)
{
	struct model *m = o->last;
	va_list ap;
	int ok;

	if (m->ncells == 0)
		return error (&o->error, "no cell to add tuple");

	va_start (ap, size);
	ok = cell_add_tuple_va (m->cell + m->ncells - 1, size, ap);
	va_end (ap);

	return ok ? 1 : error (&o->error, NULL);
}

int model_add_param (struct model *o, const char *name, const char *value)
{
	struct model *m = o->last;
	int ok;

	if (m->ncells == 0)
		return error (&o->error, "no cell to add parameter");

	ok = cell_add_param (m->cell + m->ncells - 1, name, value);

	return ok ? 1 : error (&o->error, NULL);
}

int model_add_attr (struct model *o, const char *name, const char *value)
{
	struct model *m = o->last;
	int ok;

	if (m->ncells == 0)
		return error (&o->error, "no cell to add attribute");

	ok = cell_add_attr (m->cell + m->ncells - 1, name, value);

	return ok ? 1 : error (&o->error, NULL);
}

size_t model_get_port (struct model *o, const char *name)
{
	size_t i;

	/* ToDo: create hash table to map port name to port index */

	for (i = 0; i < o->nports; ++i)
		if (strcmp (o->port[i].name, name) == 0)
			return i;

	return M_UNKNOWN;
}

struct model *model_get_model (struct model *o, const char *name)
{
	size_t i;

	/* ToDo: create hash table to map model name to model index */

	for (i = 0; i < o->nmodels; ++i)
		if (strcmp (o->model[i].name, name) == 0)
			return o->model + i;

	return NULL;
}
