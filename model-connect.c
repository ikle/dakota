/*
 * Dakota Model Connect
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

	if (!model_add_port (o, cell, name, PORT_DRIVEN | PORT_LOCAL))
		return error_s (&o->error, NULL);

	return o->nports - 1;
exists:
	if ((o->port[port].type & PORT_DRIVEN) != 0)
		return error_s (&o->error, "multiple drivers for %s", name);

	if (o->port[port].cell == NULL)
		o->port[port].cell = cell;

	o->port[port].type |= PORT_DRIVEN;
	return port;
}

static
size_t model_add_source (struct model *o, struct cell *cell, const char *name)
{
	size_t port;

	if ((port = model_get_port (o, name)) != M_UNKNOWN)
		return port;

	o->last = o;  /* add local port to this model */

	if (!model_add_port (o, cell, name, PORT_LOCAL))
		return error_s (&o->error, NULL);

	return o->nports - 1;
}

static int model_bind_wire (struct model *o, struct wire *wire)
{
	wire->to   = model_add_sink   (o, NULL, wire->sink);
	wire->from = model_add_source (o, NULL, wire->source);

	return (wire->to != M_UNKNOWN && wire->from != M_UNKNOWN);
}

static int model_bind_port (struct model *o, struct model *type,
			    struct cell *cell, size_t i, const char *expr)
{
	char *p, *l, *r;
	size_t port;
	int ok;

	if (i >= type->nports)
		return model_error (o, "too many args for cell %s", cell->type);

	if (sscanf (expr, "%m[^=]=%ms", &p, &r) == 2) {
		l = make_string ("%s.%s", cell->name, p);
		free (p);

		if ((port = model_get_port (o, l)) == M_UNKNOWN)
			goto no_port;
	}
	else {
		l = make_string ("%s.%s", cell->name, type->port[i].name);
		r = (char *) expr;
		port = i;

		if ((type->port[i].type & PORT_LOCAL) != 0)
			goto no_port;
	}

	ok = (type->port[i].type & PORT_INPUT) != 0 ?
	     model_add_wire (o, l, r):
	     model_add_wire (o, r, l);

	free (l);

	if (r != expr)
		free (r);

	return ok;
no_port:
	model_error (o, "cannot find port %s for cell %s", l, cell->type);
	free (l);

	if (r != expr)
		free (r);

	return 0;
}

static int model_bind_cell (struct model *o, struct cell *cell)
{
	struct model *m;
	size_t i, pos;
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

		port = (m->port[i].type & PORT_INPUT) != 0 ?
			model_add_source (o, cell, name):
			model_add_sink   (o, cell, name);
		free (name);

		if (port == M_UNKNOWN)
			return 0;
	}

	for (i = 0, pos = 0; i < cell->nparams; ++i)
		if (strcmp (cell->param[i].key, "dakota-bind") == 0) {
			if (!model_bind_port (o, m, cell, pos, cell->param[i].value))
				return 0;

			++pos;
		}

	return 1;
}

static int model_is_sink (struct model *o, struct port *port)
{
	if ((port->type & PORT_DRIVEN) != 0)
		return 1;

	return error (&o->error, "no driver for %s", port->name);
}

int model_connect (struct model *o)
{
	size_t i;

	for (i = 0; i < o->ncells; ++i)
		if (!model_bind_cell (o, o->cell + i))
			return 0;

	for (i = 0; i < o->nwires; ++i)
		if (!model_bind_wire (o, o->wire + i))
			return 0;

	for (i = 0; i < o->nports; ++i)
		if (!model_is_sink (o, o->port + i))
			return 0;

	for (i = 0; i < o->nmodels; ++i)
		if (!model_connect (o->model + i))
			return 0;

	return 1;
}
