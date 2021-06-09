/*
 * String Helpers
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <dakota/string.h>

static char *make_string_va (const char *fmt, va_list ap)
{
	va_list aq;
	int size;
	char *s;

	va_copy (aq, ap);
	size = vsnprintf (NULL, 0, fmt, aq) + 1;
	va_end (aq);

	if ((s = malloc (size)) == NULL)
		return NULL;

	vsnprintf (s, size, fmt, ap);
	return s;
}

char *make_string (const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	s = make_string_va (fmt, ap);
	va_end(ap);

	return s;
}
