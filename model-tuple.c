/*
 * Dakota Model Tuple
 *
 * Copyright (c) 2021-2022 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include <dakota/array.h>
#include <dakota/model/tuple.h>

int tuple_init (struct tuple *o, size_t size, va_list ap)
{
	const char *value;
	int i;

	if ((o->m = array_alloc (o->m, size)) == NULL)
		return 0;

	for (i = 0; i < size; ++i)
		if ((value = va_arg (ap, const char *)) == NULL ||
		    (o->m[i] = strdup (value)) == NULL)
			goto no_value;

	o->size = size;
	return 1;
no_value:
	array_free (o->m, i, free);
	return 0;
}

int tuple_init_v (struct tuple *o, size_t size, const char *argv[])
{
	const char *value;
	int i;

	if ((o->m = array_alloc (o->m, size)) == NULL)
		return 0;

	for (i = 0; i < size; ++i)
		if ((value = argv[i]) == NULL ||
		    (o->m[i] = strdup (value)) == NULL)
			goto no_value;

	o->size = size;
	return 1;
no_value:
	array_free (o->m, i, free);
	return 0;
}

static void tuple_entry_fini (char **entry)
{
	free (*entry);
}

void tuple_fini (struct tuple *o)
{
	array_free (o->m, o->size, tuple_entry_fini);
}
