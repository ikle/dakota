/*
 * Chip Config
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "chip-conf.h"

static int chip_error_va (struct chip_conf *o, const char *fmt, va_list ap)
{
	if (fmt == NULL)
		snprintf (o->error, sizeof (o->error), "%s", strerror (errno));
	else
		vsnprintf (o->error, sizeof (o->error), fmt, ap);

	return 0;
}

int chip_error (struct chip_conf *o, const char *fmt, ...)
{
	va_list ap;
	int ok;

	va_start(ap, fmt);
	ok = chip_error_va (o, fmt, ap);
	va_end(ap);

	return ok;
}
