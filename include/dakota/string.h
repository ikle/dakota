/*
 * String Helpers
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_STRING_H
#define DAKOTA_STRING_H  1

#include <stdarg.h>

char *make_string_va (const char *fmt, va_list ap);
char *make_string    (const char *fmt, ...);

#endif  /* DAKOTA_STRING_H */
