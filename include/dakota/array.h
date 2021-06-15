/*
 * Dakota Array
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_ARRAY_H
#define DAKOTA_ARRAY_H  1

#include <stddef.h>

void *array_do_alloc (size_t count, size_t size);

void *array_do_resize (void *o, size_t count, size_t size);
void array_do_free (void *o, size_t count, size_t size, void (*free_entry) ());

#define array_alloc(array, count) \
	array_do_alloc (count, sizeof (array[0]))

#define array_resize(array, count) \
	array_do_resize (array, count, sizeof (array[0]))

#define array_free(array, count, free_entry) \
	array_do_free (array, count, sizeof (array[0]), free_entry)

#endif /* DAKOTA_ARRAY_H */
