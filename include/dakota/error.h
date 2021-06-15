/*
 * Dakota Error
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_ERROR_H
#define DAKOTA_ERROR_H  1

#include <stddef.h>

struct error {
	int size;
	char *reason;
};

void error_init (struct error *o);
void error_fini (struct error *o);

int error (struct error *o, const char *fmt, ...);

#endif /* DAKOTA_ERROR_H */
