/*
 * Dakota Wire
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_MODEL_WIRE_H
#define DAKOTA_MODEL_WIRE_H  1

#include <stddef.h>

struct wire {
	char *sink;
	char *source;
};

int  wire_init (struct wire *o, const char *sink, const char *source);
void wire_fini (struct wire *o);

#endif  /* DAKOTA_MODEL_WIRE_H */
