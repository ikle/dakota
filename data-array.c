/*
 * Dakota Array
 *
 * Copyright (c) 2021-2022 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include <dakota/data/array.h>

void *array_do_alloc (size_t count, size_t size)
{
	if ((SIZE_MAX / size) < count) {
		errno = ENOMEM;
		return NULL;
	}

	return malloc (size * count);
}

void *array_do_resize (void *o, size_t count, size_t size)
{
	if ((SIZE_MAX / size) < count) {
		errno = ENOMEM;
		return NULL;
	}

	return realloc (o, size * count);
}

void array_do_free (void *o, size_t count, size_t size, void (*free_entry) ())
{
	size_t i, pos;

	if (free_entry != NULL)
		for (i = 0, pos = 0; i < count; ++i, pos += size)
			free_entry (o + pos);

	free (o);
}
