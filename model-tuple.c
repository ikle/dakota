/*
 * Dakota Tuple
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "model-tuple.h"

int tuple_init (struct tuple *o, int size, va_list ap)
{
	int i;

	assert (size > 0);

	if ((o->m = malloc (sizeof (o->m[0]) * size)) == NULL)
		return 0;

	for (i = 0; i < size; ++i)
		if ((o->m[i] = strdup (va_arg (ap, const char *))) == NULL)
			goto no_value;

	return 1;
no_value:
	for (; i > 0; --i)
		free (o->m + i);

	free (o->m);
	return 0;
}

void tuple_fini (struct tuple *o)
{
	int i;

	for (i = 0; i < o->size; ++i)
		free (o->m + i);

	free (o->m);
}
