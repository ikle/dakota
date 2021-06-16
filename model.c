/*
 * Dakota Model
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>

#include "model-connect.h"
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

int model_commit (struct model *o)
{
	size_t i;

	for (i = 0; i < o->nmodels; ++i)
		if (!model_connect (o->model + i))
			return 0;

	return model_connect (o);
}
