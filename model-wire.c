/*
 * Dakota Wire
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include "model-core.h"

int wire_init (struct wire *o, const char *sink, const char *source)
{
	if ((o->sink = strdup (sink)) == NULL)
		return 0;

	if ((o->source = strdup (source)) == NULL)
		goto no_source;

	o->to   = M_UNKNOWN;
	o->from = M_UNKNOWN;
	return 1;
no_source:
	free (o->sink);
	return 0;
}

void wire_fini (struct wire *o)
{
	free (o->sink);
	free (o->source);
}
