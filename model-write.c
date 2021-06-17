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

		if (value != NULL)
			ok &= fprintf (out, ".param %s %s\n", name, value) > 0;
		else
			ok &= fprintf (out, ".param %s\n", name) > 0;
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

static int cell_write_params (struct cell *o, FILE *out)
{
	const char *name, *value;
	size_t i;
	int ok = 1;

	for (i = 0; i < o->nparams; ++i) {
		name  = o->param[i].key;
		value = o->param[i].value;

		if (strcmp (name, "cname") == 0)
			ok &= fprintf (out, ".cname %s\n", value) > 0;
		else
		if (value != NULL)
			ok &= fprintf (out, ".param %s %s\n", name, value) > 0;
		else
			ok &= fprintf (out, ".param %s\n", name) > 0;
	}

	return ok;
}

static int model_write_cells (struct model *o, FILE *out)
{
	const char *type, *name;
	size_t i;
	int ok = 1;

	for (i = 0; i < o->ncells; ++i) {
		type = o->cell[i].type;
		name = o->cell[i].name;

		ok &= fprintf (out, ".cell %s %s", type, name) > 0;
		ok &= fprintf (out, "\n") > 0;
		ok &= cell_write_params (o->cell + i, out);
		ok &= fprintf (out, "\n") > 0;
	}

	return ok;
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

		ok &= fprintf (out, ".wire %s %s\n", sink, source) > 0;
	}

	if (o->nwires > 0)
		ok &= fprintf (out, "\n") > 0;

	return ok;
}

static int model_write_one (struct model *o, FILE *out)
{
	int ok = 1;

	ok &= fprintf (out, ".model %s\n", o->name) > 0;

	ok &= model_write_params  (o, out);
	ok &= model_write_inputs  (o, out);
	ok &= model_write_outputs (o, out);

	ok &= fprintf (out, "\n") > 0;

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
