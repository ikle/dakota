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

static int model_add_sink (struct model *o, const char *name)
{
	struct port *p;

	if ((p = model_get_port (o, name)) != NULL)
		goto exists;

	o->last = o;  /* add local port to this model */

	if (!model_add_port (o, NULL, name, PORT_DRIVEN | PORT_LOCAL))
		return error (&o->error, NULL);

	return 1;
exists:
	if ((p->type & PORT_DRIVEN) != 0)
		return error (&o->error, "multiple drivers for %s", name);

	p->type |= PORT_DRIVEN;
	return 1;
}

static int model_has_sink (struct model *o, struct port *port, const char *name)
{
	if (port == NULL)
		port = model_get_port (o, name);
	else
	if (name == NULL)
		name = port->name;

	if (port != NULL && (port->type & PORT_DRIVEN) != 0)
		return 1;

	return error (&o->error, "no driver for %s", name);
}

static int model_bind_cell (struct model *o, struct cell *cell)
{
	struct model *m;
	size_t i;
	char *name;
	int ok;

	if ((m = model_get_model (o, cell->type)) == NULL)
		return error (&o->error, "cannot find model %s for cell %s",
			      cell->type, cell->name);

	for (i = 0; i < m->nports; ++i) {
		if (m->port[i].type != 0)  /* is not output? */
			continue;

		name = make_string ("%s.%s", cell->name, m->port[i].name);
		if (name == NULL)
			return error (&o->error, NULL);

		ok = model_add_sink (o, name);
		free (name);

		if (!ok)
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

		ok = model_has_sink (o, NULL, name);
		free (name);

		if (!ok)
			return 0;
	}

	return 1;
}

int model_commit (struct model *o)
{
	size_t i;

	for (i = 0; i < o->ncells; ++i)
		if (!model_bind_cell (o, o->cell + i))
			return 0;

	for (i = 0; i < o->nwires; ++i)
		if (!model_add_sink (o, o->wire[i].sink))
			return 0;

	for (i = 0; i < o->nports; ++i)
		if (o->port[i].type == 0 &&  /* is output? */
		    !model_has_sink (o, o->port + i, NULL))
			return 0;

	for (i = 0; i < o->nwires; ++i)
		if (!model_has_sink (o, NULL, o->wire[i].source))
			return 0;

	for (i = 0; i < o->ncells; ++i)
		if (!model_check_cell (o, o->cell + i))
			return 0;

	return 1;
}
