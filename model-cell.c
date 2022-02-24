/*
 * Dakota Model Cell
 *
 * Copyright (c) 2021-2022 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include <dakota/data/array.h>
#include <dakota/model/cell.h>

int cell_init (struct cell *o, const char *type, const char *name)
{
	if ((o->type = strdup (type)) == NULL)
		return 0;

	if ((o->name = strdup (name)) == NULL)
		goto no_name;

	o->nbinds  = 0;
	o->nparams = 0;
	o->nattrs  = 0;
	o->ntuples = 0;

	o->bind    = NULL;
	o->param   = NULL;
	o->attr    = NULL;
	o->tuple   = NULL;

	o->map = NULL;
	return 1;
no_name:
	free (o->type);
	return 0;
}

void cell_fini (struct cell *o)
{
	free (o->type);
	free (o->name);

	array_free (o->bind,  o->nbinds,  pair_fini);
	array_free (o->param, o->nparams, pair_fini);
	array_free (o->attr,  o->nattrs,  pair_fini);
	array_free (o->tuple, o->ntuples, tuple_fini);

	bitmap_free (o->map);
}

#define DEF_PAIR_ADD(attr, count)					\
int cell_add_##attr (struct cell *o, const char *key, const char *value) \
{									\
	const size_t count = o->count + 1;				\
	struct pair *p;							\
									\
	if ((p = array_resize (o->attr, count)) == NULL)		\
		return 0;						\
									\
	o->attr = p;							\
									\
	if (!pair_init (o->attr + o->count, key, value))		\
		return 0;						\
									\
	o->count = count;						\
	return 1;							\
}

DEF_PAIR_ADD (bind,  nbinds)
DEF_PAIR_ADD (param, nparams)
DEF_PAIR_ADD (attr,  nattrs)

int cell_add_tuple_va (struct cell *o, int size, va_list ap)
{
	const size_t ntuples = o->ntuples + 1;
	struct tuple *p;

	if ((p = array_resize (o->tuple, ntuples)) == NULL)
		return 0;

	o->tuple = p;

	if (!tuple_init_va (o->tuple + o->ntuples, size, ap))
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

int cell_load_bitmap (struct cell *o, const char *path)
{
	bitmap_free (o->map);

	return (o->map = bitmap_import (path)) != NULL;
}

const char *cell_get_attr (const struct cell *o, const char *name)
{
	size_t i;

	for (i = 0; i < o->nattrs; ++i)
		if (strcmp (o->attr[i].key, name) == 0)
			return o->attr[i].value;

	return NULL;
}
