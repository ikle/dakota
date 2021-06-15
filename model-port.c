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

int port_init (struct port *o, struct cell *cell, const char *name, int type)
{
	if ((o->name = strdup (name)) == NULL)
		return 0;

	o->cell = cell;
	o->type = type;
	return 1;
}

void port_fini (struct port *o)
{
	free (o->name);
}
