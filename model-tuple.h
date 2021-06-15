/*
 * Dakota Tuple
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_MODEL_TUPLE_H
#define DAKOTA_MODEL_TUPLE_H  1

#include <stdarg.h>
#include <stddef.h>

struct tuple {
	size_t size;
	char **m;
};

int  tuple_init (struct tuple *o, size_t size, va_list ap);
void tuple_fini (struct tuple *o);

#endif  /* DAKOTA_MODEL_TUPLE_H */
