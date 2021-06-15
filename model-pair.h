/*
 * Dakota Pair
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_MODEL_PAIR_H
#define DAKOTA_MODEL_PAIR_H  1

#include <stddef.h>

struct pair {
	char *key, *value;
};

int  pair_init (struct pair *o, const char *key, const char *value);
void pair_fini (struct pair *o);

#endif  /* DAKOTA_MODEL_PAIR_H */
