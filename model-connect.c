/*
 * Dakota Model Connect
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>

#include <dakota/string.h>

#include "model-connect.h"
#include "model-core.h"

static
size_t model_add_sink (struct model *o, struct cell *cell, const char *name)
{
	size_t port;

	if ((port = model_get_port (o, name)) != M_UNKNOWN)
		goto exists;

	o->last = o;  /* add local port to this model */

	if (!model_add_port (o, cell, name, PORT_DRIVEN | PORT_LOCAL)) {
		error (&o->error, NULL);
		return M_UNKNOWN;
	}

	return o->nports - 1;
exists:
	if ((o->port[port].type & PORT_DRIVEN) != 0) {
		error (&o->error, "multiple drivers for %s", name);
		return M_UNKNOWN;
	}

	o->port[port].cell  = cell;
	o->port[port].type |= PORT_DRIVEN;
	return port;
}

static size_t model_add_source (struct model *o, const char *name)
{
	size_t port;

	if ((port = model_get_port (o, name)) != M_UNKNOWN)
		return port;

	o->last = o;  /* add local port to this model */

	if (!model_add_port (o, NULL, name, PORT_LOCAL)) {
		error (&o->error, NULL);
		return M_UNKNOWN;
	}

	return o->nports - 1;
}

static size_t model_get_sink (struct model *o, size_t port, const char *name)
{
	if (port == M_UNKNOWN)
		port = model_get_port (o, name);
	else
	if (name == NULL)
		name = o->port[port].name;

	if (port != M_UNKNOWN && (o->port[port].type & PORT_DRIVEN) != 0)
		return port;

	error (&o->error, "no driver for %s", name);
	return M_UNKNOWN;
}

static int model_has_sink (struct model *o, size_t port, const char *name)
{
	return (model_get_sink (o, port, name) != M_UNKNOWN);
}

static int model_bind_wire (struct model *o, struct wire *wire)
{
	wire->to = model_add_sink (o, NULL, wire->sink);

	return (wire->to != M_UNKNOWN);
}

static int model_bind_cell (struct model *o, struct cell *cell)
{
	struct model *m;
	size_t i;
	char *name;
	size_t port;

	if ((m = model_get_model (o, cell->type)) == NULL)
		return error (&o->error, "cannot find model %s for cell %s",
			      cell->type, cell->name);

	for (i = 0; i < m->nports; ++i) {
		if ((m->port[i].type & PORT_LOCAL) != 0)
			continue;

		name = make_string ("%s.%s", cell->name, m->port[i].name);
		if (name == NULL)
			return error (&o->error, NULL);

		port = (m->port[i].type & PORT_DRIVEN) != 0 ?
			model_add_source (o, name):
			model_add_sink (o, cell, name);
		free (name);

		if (port == M_UNKNOWN)
			return 0;
	}

	return 1;
}

static int model_check_cell (struct model *o, struct cell *cell)
{
	struct model *m;
	size_t i;
	char *name;
	int ok;

	if ((m = model_get_model (o, cell->type)) == NULL)
		return error (&o->error, "cannot find model %s for cell %s",
			      cell->type, cell->name);

	for (i = 0; i < m->nports; ++i) {
		if (m->port[i].type != PORT_DRIVEN)  /* is not input? */
			continue;

		name = make_string ("%s.%s", cell->name, m->port[i].name);
		if (name == NULL)
			return error (&o->error, NULL);

		ok = model_has_sink (o, M_UNKNOWN, name);
		free (name);

		if (!ok)
			return 0;
	}

	return 1;
}

int model_connect (struct model *o)
{
	size_t i;

	for (i = 0; i < o->nwires; ++i)
		if (!model_bind_wire (o, o->wire + i))
			return 0;

	for (i = 0; i < o->ncells; ++i)
		if (!model_bind_cell (o, o->cell + i))
			return 0;

	for (i = 0; i < o->nports; ++i)
		if (o->port[i].type == 0 &&  /* is output? */
		    !model_has_sink (o, i, NULL))
			return 0;

	for (i = 0; i < o->nwires; ++i) {
		o->wire[i].from = model_get_sink (o, M_UNKNOWN,
						  o->wire[i].source);
		if (o->wire[i].from == M_UNKNOWN)
			return 0;
	}

	for (i = 0; i < o->ncells; ++i)
		if (!model_check_cell (o, o->cell + i))
			return 0;

	for (i = 0; i < o->nmodels; ++i)
		if (!model_connect (o->model + i))
			return 0;

	return 1;
}
