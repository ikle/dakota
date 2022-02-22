/*
 * Dakota Pair
 *
 * Copyright (c) 2021-2022 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include <dakota/model/pair.h>

int pair_init (struct pair *o, const char *key, const char *value)
{
	if ((o->value = strdup (value)) == NULL)
		return 0;

	if (key == NULL)
		o->key = NULL;
	else
	if ((o->key = strdup (key)) == NULL)
		goto no_key;

	return 1;
no_key:
	free (o->value);
	return 0;
}

void pair_fini (struct pair *o)
{
	free (o->key);
	free (o->value);
}
