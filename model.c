/*
 * Dakota Model
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>

#include <dakota/error.h>

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

int model_error (struct model *o, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	error_va (&o->error, fmt, ap);
	va_end(ap);

	return 0;
}

const char *model_status (struct model *o)
{
	return o->error.reason;
}

int model_commit (struct model *o)
{
	return model_connect (o);
}
