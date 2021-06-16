/*
 * Dakota Cell
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include <dakota/array.h>

#include "model-cell.h"

int cell_init (struct cell *o, const char *type, const char *name)
{
	if ((o->type = strdup (type)) == NULL)
		return 0;

	if ((o->name = strdup (name)) == NULL)
		goto no_name;

	o->ntuples = 0;
	o->tuple   = NULL;
	o->nparams = 0;
	o->param   = NULL;
	o->nattrs  = 0;
	o->attr    = NULL;
	return 1;
no_name:
	free (o->type);
	return 0;
}

void cell_fini (struct cell *o)
{
	free (o->type);
	free (o->name);

	array_free (o->tuple, o->ntuples, tuple_fini);
	array_free (o->param, o->nparams, pair_fini);
	array_free (o->attr,  o->nattrs,  pair_fini);
}

int cell_add_tuple_va (struct cell *o, int size, va_list ap)
{
	const size_t ntuples = o->ntuples + 1;
	struct tuple *p;

	if ((p = array_resize (o->tuple, ntuples)) == NULL)
		return 0;

	o->tuple = p;

	if (!tuple_init (o->tuple + o->ntuples, size, ap))
		return 0;

	o->ntuples = ntuples;
	return 1;
}

int cell_add_tuple_v (struct cell *o, int size, const char *argv[])
{
	const size_t ntuples = o->ntuples + 1;
	struct tuple *p;

	if ((p = array_resize (o->tuple, ntuples)) == NULL)
		return 0;

	o->tuple = p;

	if (!tuple_init_v (o->tuple + o->ntuples, size, argv))
		return 0;

	o->ntuples = ntuples;
	return 1;
}

int cell_add_tuple (struct cell *o, int size, ...)
{
	va_list ap;
	int ok;

	va_start (ap, size);
	ok = cell_add_tuple_va (o, size, ap);
	va_end (ap);

	return ok;
}

int cell_add_param (struct cell *o, const char *name, const char *value)
{
	const size_t nparams = o->nparams + 1;
	struct pair *p;

	if ((p = array_resize (o->param, nparams)) == NULL)
		return 0;

	o->param = p;

	if (!pair_init (o->param + o->nparams, name, value))
		return 0;

	o->nparams = nparams;
	return 1;
}

int cell_add_attr  (struct cell *o, const char *name, const char *value)
{
	const size_t nattrs = o->nattrs + 1;
	struct pair *p;

	if ((p = array_resize (o->attr, nattrs)) == NULL)
		return 0;

	o->attr = p;

	if (!pair_init (o->attr + o->nattrs, name, value))
		return 0;

	o->nattrs = nattrs;
	return 1;
}
