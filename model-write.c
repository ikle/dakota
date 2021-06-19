/*
 * Dakota Model Writer
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <string.h>

#include <dakota/model.h>

#include "model-core.h"

static int model_write_params (struct model *o, FILE *out)
{
	const char *name, *value;
	size_t i;
	int ok = 1;

	for (i = 0; i < o->nparams; ++i) {
		name  = o->param[i].key;
		value = o->param[i].value;

		ok &= (value != NULL) ?
		      fprintf (out, ".param %s %s\n", name, value) > 0 :
		      fprintf (out, ".param %s\n",    name) > 0;
	}

	return ok;
}

static int model_write_inputs (struct model *o, FILE *out)
{
	const char *prefix, *name;
	size_t i;
	int ok = 1;

	for (i = 0, prefix = ".inputs "; i < o->nports; ++i)
		if ((o->port[i].type & PORT_INPUT) != 0) {
			name = o->port[i].name;
			ok &= fprintf (out, "%s%s", prefix, name) > 0;
			prefix = " ";
		}

	if (prefix[0] == ' ')
		ok &= fprintf (out, "\n") > 0;

	return ok;
}

static int model_write_outputs (struct model *o, FILE *out)
{
	const char *prefix, *name;
	size_t i;
	int ok = 1;

	for (i = 0, prefix = ".outputs "; i < o->nports; ++i)
		if ((o->port[i].type & (PORT_INPUT | PORT_LOCAL)) == 0) {
			name = o->port[i].name;
			ok &= fprintf (out, "%s%s", prefix, name) > 0;
			prefix = " ";
		}

	if (prefix[0] == ' ')
		ok &= fprintf (out, "\n") > 0;

	return ok;
}

static int cell_write_binds (struct cell *o, FILE *out)
{
	size_t i;
	int ok = 1;

	for (i = 0; i < o->nattrs; ++i)
		if (strcmp (o->attr[i].key, "cell-bind") == 0)
			ok &= fprintf (out, " %s", o->attr[i].value) > 0;

	ok &= fprintf (out, "\n") > 0;
	return ok;
}

static int cell_write_attrs (struct cell *o, FILE *out)
{
	const char *name, *value;
	size_t i;
	int ok = 1;

	for (i = 0; i < o->nattrs; ++i) {
		name  = o->attr[i].key;
		value = o->attr[i].value;

		if (strncmp (name, "cell-", 5) == 0)
			continue;

		if (strcmp (name, "cname") == 0)
			ok &= fprintf (out, ".cname %s\n", value) > 0;
		else
		if (value != NULL)
			ok &= fprintf (out, ".attr %s %s\n", name, value) > 0;
		else
			ok &= fprintf (out, ".attr %s\n", name) > 0;
	}

	return ok;
}

static int cell_write_params (struct cell *o, FILE *out)
{
	const char *name, *value;
	size_t i;
	int ok = 1;

	for (i = 0; i < o->nparams; ++i) {
		name  = o->param[i].key;
		value = o->param[i].value;

		if (value != NULL)
			ok &= fprintf (out, ".param %s %s\n", name, value) > 0;
		else
			ok &= fprintf (out, ".param %s\n", name) > 0;
	}

	return ok;
}

static int cell_write_tuples (struct cell *o, FILE *out)
{
	size_t i, j;
	const char *sep;
	const struct tuple *tuple;
	int ok = 1;

	for (i = 0; i < o->ntuples; ++i) {
		tuple = o->tuple + i;

		for (j = 0, sep = ""; j < tuple->size; ++j, sep = "\t")
			ok &= fprintf (out, "%s%s", sep, tuple->m[j]) > 0;

		ok &= fprintf (out, "\n") > 0;
	}

	return ok;
}

static const char *cell_get_kind (struct cell *o)
{
	const char *name, *value;
	size_t i;

	for (i = 0; i < o->nattrs; ++i) {
		name  = o->attr[i].key;
		value = o->attr[i].value;

		if (strcmp (name, "cell-kind") == 0)
			return value;
	}

	return "subckt";
}

static int model_write_latch (struct model *o, struct cell *c, FILE *out)
{
	const char *type = "re", *a[3], *init = NULL;
	size_t i, j;
	int ok = 1;

	for (i = 0, j = 0; i < c->nattrs; ++i)
		if (strcmp (c->attr[i].key, "cell-edge") == 0)
			type = c->attr[i].value;
		else
		if (strcmp (c->attr[i].key, "cell-bind") == 0 && j < 3) {
			a[j++] = c->attr[i].value;
		}
		else
		if (strcmp (c->attr[i].key, "cell-init") == 0)
			init = c->attr[i].value;

	if (j > 2)
		ok &= fprintf (out, ".latch %s %s %s %s",
			       a[1], a[2], type, a[0]) > 0;
	else
		ok &= fprintf (out, ".latch %s %s", a[0], a[1]) > 0;

	if (init != NULL)
		ok &= fprintf (out, " %s\n", init) > 0;
	else
		ok &= fprintf (out, "\n") > 0;

	ok &= cell_write_attrs  (c, out);
	ok &= cell_write_params (c, out);
	ok &= cell_write_tuples (c, out);

	return ok;
}

static int model_write_cell (struct model *o, size_t i, FILE *out)
{
	const char *kind, *type;
	int ok = 1;

	kind = cell_get_kind (o->cell + i);
	type = o->cell[i].type;

	if (strcmp (type, "latch") == 0)
		return model_write_latch (o, o->cell + i, out);

	if (strcmp (type, "table") == 0)
		ok &= fprintf (out, ".%s", kind) > 0;
	else
		ok &= fprintf (out, ".%s %s", kind, type) > 0;

	ok &= cell_write_binds  (o->cell + i, out);
	ok &= cell_write_attrs  (o->cell + i, out);
	ok &= cell_write_params (o->cell + i, out);
	ok &= cell_write_tuples (o->cell + i, out);

	return ok;
}

static int model_write_cells (struct model *o, FILE *out)
{
	size_t i;
	int ok = 1;

	for (i = 0; i < o->ncells; ++i)
		ok &= model_write_cell (o, i, out);

	return ok;
}

static int port_is_internal (const struct port *p)
{
	return (p->type & PORT_LOCAL) != 0 && p->cell == NULL;
}

static int model_write_wires (struct model *o, FILE *out)
{
	const char *sink, *source;
	size_t i, to, from;
	int ok = 1;

	for (i = 0; i < o->nwires; ++i) {
		sink   = o->wire[i].sink;
		source = o->wire[i].source;
		to     = o->wire[i].to;
		from   = o->wire[i].from;

		if (to == M_UNKNOWN || from == M_UNKNOWN)
			return model_error (o, "broken wire from %s to %s",
					    source, sink);

		if (port_is_internal (o->port + to) &&
		    port_is_internal (o->port + from))
			ok &= fprintf (out, ".wire %s %s\n", sink, source) > 0;
	}

	return ok;
}

static int model_write_one (struct model *o, FILE *out)
{
	int ok = 1;

	ok &= fprintf (out, ".model %s\n", o->name) > 0;

	ok &= model_write_params  (o, out);
	ok &= model_write_inputs  (o, out);
	ok &= model_write_outputs (o, out);
	ok &= model_write_cells   (o, out);
	ok &= model_write_wires   (o, out);

	ok &= fprintf (out, ".end\n") > 0;

	return ok;
}

int model_write (struct model *o, const char *path)
{
	FILE *out;
	size_t i;

	if (strcmp (path, "-") == 0)
		out = stdout;
	else
	if ((out = fopen (path, "w")) == NULL)
		return model_error (o, NULL);

	if (!model_write_one (o, out))
		goto error;

	for (i = 0; i < o->nmodels; ++i) {
		fprintf (out, "\n");

		if (!model_write_one (o->model + i, out))
			goto error;
	}

	if (fclose (out) != 0)
		return model_error (o, NULL);

	return 1;
error:
	fclose (out);

	if (strcmp (path, "-") != 0)
		remove (path);

	return 0;
}
