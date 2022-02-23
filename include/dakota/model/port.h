/*
 * Dakota Model Port
 *
 * Copyright (c) 2021-2022 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_MODEL_PORT_H
#define DAKOTA_MODEL_PORT_H  1

#include <stddef.h>

enum port_type {
	PORT_SOURCE	= 1,	/* port is source, sink otherwise	*/
	PORT_DRIVEN	= 2,	/* port has driver: connected to source	*/
	PORT_LOCAL	= 4,
};

struct port {
	char *name;
	int type;

	struct cell *cell;	/* binded cell      */
	size_t ref;		/* binded cell port */
};

int  port_init (struct port *o, const char *name, int type,
		struct cell *cell, size_t ref);
void port_fini (struct port *o);

#endif  /* DAKOTA_MODEL_PORT_H */
