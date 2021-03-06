/*
 * Dakota Error
 *
 * Copyright (c) 2021-2022 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dakota/error.h>

void error_init (struct error *o)
{
	o->size   = 0;
	o->reason = NULL;
}

void error_fini (struct error *o)
{
	free (o->reason);
}

void error_move (struct error *o, struct error *from)
{
	error_fini (o);

	*o = *from;

	error_init (from);
}

static int error_print_va (struct error *o, const char *fmt, va_list ap)
{
	if (fmt == NULL)
		return snprintf (o->reason, o->size, "%s", strerror (errno));

	return vsnprintf (o->reason, o->size, fmt, ap);
}

int error_va (struct error *o, const char *fmt, va_list ap)
{
	va_list aq;
	int len;
	char *p;

	va_copy (aq, ap);
	len = error_print_va (o, fmt, aq);
	va_end (aq);

	if (len < o->size)
		return 0;

	if ((p = realloc (o->reason, len + 1)) == NULL)
		return 0;

	o->reason = p;
	o->size   = len + 1;

	error_print_va (o, fmt, ap);
	return 0;
}

int error (struct error *o, const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	error_va (o, fmt, ap);
	va_end (ap);

	return 0;
}

size_t error_s (struct error *o, const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	error_va (o, fmt, ap);
	va_end (ap);

	return (size_t) -1;
}

void *error_p (struct error *o, const char *fmt, ...)
{
	va_list ap;

	va_start (ap, fmt);
	error_va (o, fmt, ap);
	va_end (ap);

	return NULL;
}
