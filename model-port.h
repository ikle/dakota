/*
 * Dakota Port
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_MODEL_PORT_H
#define DAKOTA_MODEL_PORT_H  1

#include <stddef.h>

enum port_type {
	PORT_SOURCE	= 0,
	PORT_SINK	= 1,
	PORT_LOCAL	= 2,
};

struct port {
	/* cell = NULL for model ports and local connection points */
	struct cell *cell;
	char *name;
	int type;
};

int  port_init (struct port *o, struct cell *cell, const char *name, int type);
void port_fini (struct port *o);

#endif  /* DAKOTA_MODEL_PORT_H */
