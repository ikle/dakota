/*
 * Dakota Tuple
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include <dakota/array.h>

#include "model-tuple.h"

int tuple_init (struct tuple *o, size_t size, va_list ap)
{
	int i;

	if ((o->m = array_alloc (o->m, size)) == NULL)
		return 0;

	for (i = 0; i < size; ++i)
		if ((o->m[i] = strdup (va_arg (ap, const char *))) == NULL)
			goto no_value;

	return 1;
no_value:
	array_free (o->m, i, free);
	return 0;
}

int tuple_init_v (struct tuple *o, size_t size, const char *argv[])
{
	int i;

	if ((o->m = array_alloc (o->m, size)) == NULL)
		return 0;

	for (i = 0; i < size; ++i)
		if ((o->m[i] = strdup (argv[i])) == NULL)
			goto no_value;

	return 1;
no_value:
	array_free (o->m, i, free);
	return 0;
}

void tuple_fini (struct tuple *o)
{
	array_free (o->m, o->size, free);
}
