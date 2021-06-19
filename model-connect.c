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

static size_t model_add_sink (struct model *o, const char *name,
			      struct cell *cell, size_t ref)
{
	size_t port;

	if ((port = model_get_port (o, name)) != M_UNKNOWN)
		goto exists;

	o->last = o;  /* add local port to this model */

	if (!model_add_port (o, name, PORT_DRIVEN | PORT_LOCAL, cell, ref))
		return error_s (&o->error, NULL);

	return o->nports - 1;
exists:
	if ((o->port[port].type & PORT_DRIVEN) != 0)
		return error_s (&o->error, "multiple drivers for %s", name);

	if (o->port[port].cell == NULL) {
		o->port[port].cell = cell;
		o->port[port].ref  = ref;
	}

	o->port[port].type |= PORT_DRIVEN;
	return port;
}

static size_t model_add_source (struct model *o, const char *name,
				struct cell *cell, size_t ref)
{
	size_t port;

	if ((port = model_get_port (o, name)) != M_UNKNOWN)
		return port;

	o->last = o;  /* add local port to this model */

	if (!model_add_port (o, name, PORT_LOCAL, cell, ref))
		return error_s (&o->error, NULL);

	return o->nports - 1;
}

static int model_bind_param (struct model *o, struct pair *param)
{
	size_t i, port;
	char *name;

	if (strlen (param->value) == 1)
		return model_add_sink (o, param->key, NULL, 0) != M_UNKNOWN;

	for (i = 0; param->value[i] != '\0'; ++i) {
		name = make_string ("%s[%zu]", param->key, i);
		if (name == NULL)
			return model_error (o, NULL);

		port = model_add_sink (o, name, NULL, 0);;
		free (name);

		if (port == M_UNKNOWN)
			return 0;
	}

	return 1;
}

static int model_bind_wire (struct model *o, struct wire *wire)
{
	wire->to   = model_add_sink   (o, wire->sink,   NULL, 0);
	wire->from = model_add_source (o, wire->source, NULL, 0);

	return (wire->to != M_UNKNOWN && wire->from != M_UNKNOWN);
}

static int model_bind_core (struct model *o, struct cell *cell)
{
	size_t i, ref, ni, no, port;
	const char *name;

	for (
		i = 0, ref = 0, ni = cell->ni, no = cell->no;
		i < cell->nattrs;
		++i
	)
		if (strcmp (cell->attr[i].key, "cell-bind") == 0) {
			name = cell->attr[i].value;

			if (ni > 0) {
				port = model_add_source (o, name, cell, ref);
				--ni;
			}
			else if (no > 0) {
				port = model_add_sink (o, name, cell, ref);
				--no;
			}
			else
				goto no_ports;

			if (port == M_UNKNOWN)
				return 0;

			++ref;
		}

	return 1;
no_ports:
	return model_error (o, "too many binds for core cell %s", cell->name);
}

static int model_bind_port (struct model *o, const char *bind,
			    struct model *type, struct cell *cell, size_t ref)
{
	char *name, fake;
	size_t port;

	if (ref >= type->nports)
		return model_error (o, "too many args for cell %s", cell->type);

	if (sscanf (bind, "%m[^=]=%c", &name, &fake) == 2) {
		if ((ref = model_get_port (type, name)) == M_UNKNOWN)
			goto no_ref;

		free (name);
		bind = strchr (bind, '=') + 1;
	}

	if ((type->port[ref].type & PORT_LOCAL) != 0)
		return model_error (o, "cannot bind %s to local port of "
				    "cell %s", bind, cell->type);

	port = (type->port[ref].type & PORT_INPUT) != 0 ?
	       model_add_source (o, bind, cell, ref):
	       model_add_sink   (o, bind, cell, ref);

	return port != M_UNKNOWN;
no_ref:
	model_error (o, "cannot find port %s for cell %s", name, cell->type);
	free (name);
	return 0;
}

static int model_bind_cell (struct model *o, struct cell *cell)
{
	struct model *type;
	size_t i, ref;
	const char *bind;

	if (strcmp (cell->type, "table") == 0 ||
	    strcmp (cell->type, "latch") == 0)
		return model_bind_core (o, cell);

	if ((type = model_get_model (o, cell->type)) == NULL)
		return error (&o->error, "cannot find model %s for cell %s",
			      cell->type, cell->name);

	for (i = 0, ref = 0; i < cell->nattrs; ++i)
		if (strcmp (cell->attr[i].key, "cell-bind") == 0) {
			bind = cell->attr[i].value;

			if (!model_bind_port (o, bind, type, cell, ref))
				return 0;

			++ref;
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

	for (i = 0; i < o->nparams; ++i)
		if (!model_bind_param (o, o->param + i))
			return 0;

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
		if (!model_connect (o->model + i)) {
			error_move (&o->error, &o->model[i].error);
			return 0;
		}

	return 1;
}
