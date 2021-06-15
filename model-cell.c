/*
 * Dakota Cell
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

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
	size_t i;

	free (o->type);
	free (o->name);

	for (i = 0; i < o->ntuples; ++i)
		tuple_fini (o->tuple + i);

	free (o->tuple);

	for (i = 0; i < o->nparams; ++i)
		pair_fini (o->param + i);

	free (o->param);

	for (i = 0; i < o->nattrs; ++i)
		pair_fini (o->attr + i);

	free (o->attr);
}

int cell_add_tuple (struct cell *o, int size, ...)
{
	const size_t ntuples = o->ntuples + 1;
	struct tuple *p;
	va_list ap;
	int ok;

	if ((p = realloc (o->tuple, sizeof (p[0]) * ntuples)) == NULL)
		return 0;

	o->tuple = p;

	va_start (ap, size);
	ok = tuple_init (o->tuple + o->ntuples, size, ap);
	va_end (ap);

	if (!ok)
		return 0;

	o->ntuples = ntuples;
	return 1;
}

int cell_add_param (struct cell *o, const char *name, const char *value)
{
	const size_t nparams = o->nparams + 1;
	struct pair *p;

	if ((p = realloc (o->param, sizeof (p[0]) * nparams)) == NULL)
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

	if ((p = realloc (o->attr, sizeof (p[0]) * nattrs)) == NULL)
		return 0;

	o->attr = p;

	if (!pair_init (o->attr + o->nattrs, name, value))
		return 0;

	o->nattrs = nattrs;
	return 1;
}
