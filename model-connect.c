/*
 * Dakota Model Connect
 *
 * Copyright (c) 2021-2022 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dakota/string.h>

#include "model-connect.h"
#include "model-core.h"

/*
 * Add new driven local port or mark existing port driven
 */
static int model_add_driven (struct model *o, const char *name,
			     struct cell *cell, size_t ref)
{
	size_t port;

	if ((port = model_get_port (o, name)) != M_UNKNOWN)
		goto exists;

	o->last = o;  /* add local port to this model */

	if (!model_add_port (o, name, PORT_DRIVEN | PORT_LOCAL, cell, ref))
		return error (&o->error, NULL);

	return 1;
exists:
	if ((o->port[port].type & PORT_DRIVEN) != 0)
		return error (&o->error, "multiple drivers for %s", name);

	if (cell != NULL) {
		o->port[port].cell = cell;
		o->port[port].ref  = ref;
	}

	o->port[port].type |= PORT_DRIVEN;
	return 1;
}

/*
 * Add local porl
 */
static int model_add_local (struct model *o, const char *name,
			    struct cell *cell, size_t ref)
{
	if (model_get_port (o, name) != M_UNKNOWN)
		return 1;

	o->last = o;  /* add local port to this model */

	if (!model_add_port (o, name, PORT_LOCAL, cell, ref))
		return error (&o->error, NULL);

	return 1;
}

static int model_bind_param (struct model *o, struct pair *param)
{
	size_t i;
	char *name;
	int ok;

	if (strlen (param->value) == 1)
		return model_add_driven (o, param->key, NULL, 0);

	for (i = 0; param->value[i] != '\0'; ++i) {
		name = make_string ("%s[%zu]", param->key, i);
		if (name == NULL)
			return model_error (o, NULL);

		ok = model_add_driven (o, name, NULL, 0);
		free (name);

		if (!ok)
			return 0;
	}

	return 1;
}

static int model_bind_table (struct model *o, struct cell *cell)
{
	size_t ref, ninputs;
	const char *name;
	int ok;

	for (
		ref = 0, ninputs = cell->nbinds - 1;
		ref < cell->nbinds;
		++ref
	) {
		name = cell->bind[ref].value;

		if (strcmp (name, "->") == 0) {
			ninputs = ref;
			continue;
		}

		ok = (ref < ninputs) ?
		     model_add_local  (o, name, cell, ref):
		     model_add_driven (o, name, cell, ref);

		if (!ok)
			return 0;
	}

	return 1;
}

static int model_bind_latch (struct model *o, struct cell *c)
{
	int ok = 1;

	switch (c->nbinds) {
	case 5:
		/* init-val at index 4 */
	case 4:
		ok &= model_add_local  (o, c->bind[3].value, c, 3);
	case 3:
		/* init-val at index 2 */
	case 2:
		ok &= model_add_local  (o, c->bind[0].value, c, 0);
		ok &= model_add_driven (o, c->bind[1].value, c, 1);
		break;
	default:
		return model_error (o, "wrong number of arguments for latch");
	}

	return ok;
}

static int model_bind_port (struct model *o, const char *name, const char *bind,
			    struct model *type, struct cell *cell, size_t ref)
{
	int ok;

	if (ref >= type->nports)
		return model_error (o, "too many args for cell %s", cell->type);

	if (name != NULL &&
	    (ref = model_get_port (type, name)) == M_UNKNOWN)
		return model_error (o, "cannot find port %s for cell %s",
				    name, cell->type);

	if ((type->port[ref].type & PORT_LOCAL) != 0)
		return model_error (o, "cannot bind %s to local port of "
				    "cell %s", bind, cell->type);

	ok = (type->port[ref].type & PORT_INPUT) != 0 ?
	     model_add_local  (o, bind, cell, ref):
	     model_add_driven (o, bind, cell, ref);

	return ok;
}

static int model_bind_cell (struct model *o, struct cell *cell)
{
	struct model *type;
	size_t ref;
	const char *port, *value;

	if (strcmp (cell->type, "table") == 0)
		return model_bind_table (o, cell);

	if (strcmp (cell->type, "latch") == 0)
		return model_bind_latch (o, cell);

	if ((type = model_get_model (o, cell->type)) == NULL)
		return error (&o->error, "cannot find model %s for cell %s",
			      cell->type, cell->name);

	for (ref = 0; ref < cell->nbinds; ++ref) {
		port  = cell->bind[ref].key;
		value = cell->bind[ref].value;

		if (!model_bind_port (o, port, value, type, cell, ref))
			return 0;
	}

	return 1;
}

static int model_port_is_driven (struct model *o, struct port *port)
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

	for (i = 0; i < o->nports; ++i)
		if (!model_port_is_driven (o, o->port + i))
			return 0;

	for (i = 0; i < o->nmodels; ++i)
		if (!model_connect (o->model + i)) {
			error_move (&o->error, &o->model[i].error);
			return 0;
		}

	return 1;
}
