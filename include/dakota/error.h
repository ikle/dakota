/*
 * Dakota Error
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_ERROR_H
#define DAKOTA_ERROR_H  1

#include <stdarg.h>
#include <stddef.h>

struct error {
	int size;
	char *reason;
};

void error_init (struct error *o);
void error_fini (struct error *o);

int error_va (struct error *o, const char *fmt, va_list ap);

int    error   (struct error *o, const char *fmt, ...);
size_t error_s (struct error *o, const char *fmt, ...);
void  *error_p (struct error *o, const char *fmt, ...);

#endif /* DAKOTA_ERROR_H */
