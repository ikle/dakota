/*
 * Dakota Port
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include "model-port.h"

int port_init (struct port *o, const char *name, int type,
	       struct cell *cell, size_t ref)
{
	if ((o->name = strdup (name)) == NULL)
		return 0;

	o->type = type;
	o->cell = cell;
	o->ref  = ref;
	return 1;
}

void port_fini (struct port *o)
{
	free (o->name);
}
