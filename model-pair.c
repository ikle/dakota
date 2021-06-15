/*
 * Dakota Pair
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include "model-pair.h"

int pair_init (struct pair *o, const char *key, const char *value)
{
	if ((o->key = strdup (key)) == NULL)
		return 0;

	if ((o->value = strdup (value)) == NULL)
		goto no_value;

	return 1;
no_value:
	free (o->key);
	return 0;
}

void pair_fini (struct pair *o)
{
	free (o->key);
	free (o->value);
}
