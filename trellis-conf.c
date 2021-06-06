/*
 * Trellis Config Parser
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chip-bits.h"
#include "trellis-conf.h"

static int conf_error_va (struct chip_conf *o, const char *fmt, va_list ap)
{
	vsnprintf (o->error, sizeof (o->error), fmt, ap);
	return 0;
}

static int conf_error (struct chip_conf *o, const char *fmt, ...)
{
	va_list ap;
	int ok;

	va_start(ap, fmt);
	ok = conf_error_va (o, fmt, ap);
	va_end(ap);

	return ok;
}

static int next_ns (FILE *in)
{
	char la;

	for (;;) {
		if (fscanf (in, " %c", &la) != 1)
			return EOF;

		if (la != '#')
			break;

		fscanf (in, "%*[^\n]");
	}

	ungetc (la, in);
	return la;
}

static int next_entry (FILE *in)
{
	int la = next_ns (in);

	return (la != EOF);
}

static int next_record (FILE *in)
{
	int la = next_ns (in);

	return (la != EOF && la != '.');
}

static int match (const char *a, const char *b)
{
	return strcmp (a, b) == 0;
}

static int read_device (struct chip_conf *o, FILE *in)
{
	char *name;
	int ok;

	if (fscanf (in, "%*[ \t]%ms", &name) != 1)
		return conf_error (o, "device name required");

	ok = o->action->on_device (o->cookie, name);
	free (name);
	return ok;
}

static int read_comment (struct chip_conf *o, FILE *in)
{
	char *value;
	int ok;

	if (fscanf (in, "%*[ \t]%m[^\n]", &value) != 1)
		return conf_error (o, "empty comment");

	ok = o->action->on_comment (o->cookie, value);
	free (value);
	return ok;
}

static int read_sysconfig (struct chip_conf *o, FILE *in)
{
	char *name, *value;
	int ok;

	if (fscanf (in, "%*[ \t]%ms%*[ \t]%ms", &name, &value) != 2)
		return conf_error (o, "sysconfig requres name and value");

	ok = o->action->on_sysconfig (o->cookie, name, value);
	free (name);
	free (value);
	return ok;
}

static int read_raw (struct chip_conf *o, FILE *in, int top)
{
	int bit, ok;

	if ((bit = chip_bit_read (in)) < 0)
		return conf_error (o, "raw (unknown) requires chip bit");

	ok = o->action->on_raw (o->cookie, bit);
	return (ok && top) ? o->action->on_commit (o->cookie) : ok;
}

static int read_arrow (struct chip_conf *o, FILE *in, int top)
{
	char *sink, *source;
	int ok;

	if (fscanf (in, "%*[ \t]%ms%*[ \t]%ms", &sink, &source) != 2)
		return conf_error (o, "arrow (arc) requires sink and source");

	ok = o->action->on_arrow (o->cookie, sink, source);
	free (source);
	free (sink);
	return (ok && top) ? o->action->on_commit (o->cookie) : ok;
}

static int read_mux_conf (struct chip_conf *o, FILE *in)
{
	char *source;
	unsigned *bits;
	int ok = 1;

	while (ok && next_record (in)) {
		if (fscanf (in, "%ms", &source) != 1)
			return conf_error (o, "source name required");

		if ((bits = chip_bits_read (in)) == NULL && errno != 0)
			goto no_bits;

		ok = o->action->on_mux_data (o->cookie, source, bits);
		free (source);
		free (bits);
	}

	return ok ? o->action->on_commit (o->cookie) : 0;
no_bits:
	free (source);
	return conf_error (o, "chip bits required");
}

static int read_mux (struct chip_conf *o, FILE *in)
{
	char *name;
	int ok;

	if (fscanf (in, "%*[ \t]%ms", &name) != 1)
		return conf_error (o, "mux name required");

	ok = o->action->on_mux (o->cookie, name);
	free (name);
	return ok ? read_mux_conf (o, in) : 0;
}

static int read_word_conf (struct chip_conf *o, FILE *in)
{
	unsigned *bits;
	int ok = 1;

	while (ok && next_record (in)) {
		if ((bits = chip_bits_read (in)) == NULL && errno != 0)
			return conf_error (o, "chip bits required");

		ok = o->action->on_word_data (o->cookie, bits);
		free (bits);
	}

	return ok ? o->action->on_commit (o->cookie) : 0;
}

static int read_word (struct chip_conf *o, FILE *in, int top)
{
	char *name, *value;
	int ok;

	if (fscanf (in, "%*[ \t]%ms%*[ \t]%ms", &name, &value) != 2)
		return conf_error (o, "word requires name and value");

	ok = o->action->on_word (o->cookie, name, value);
	free (name);
	free (value);
	return (ok && top) ? read_word_conf (o, in) : ok;
}

static int read_enum_conf (struct chip_conf *o, FILE *in)
{
	char *value;
	unsigned *bits;
	int ok = 1;

	while (ok && next_record (in)) {
		if (fscanf (in, "%ms", &value) != 1)
			return conf_error (o, "value name required");

		if ((bits = chip_bits_read (in)) == NULL && errno != 0)
			goto no_bits;

		ok = o->action->on_enum_data (o->cookie, value, bits);
		free (value);
		free (bits);
	}

	return ok ? o->action->on_commit (o->cookie) : 0;
no_bits:
	free (value);
	return conf_error (o, "chip bits required");
}

static int read_enum (struct chip_conf *o, FILE *in, int top)
{
	char *name, *value;
	int ok;

	if (fscanf (in, "%*[ \t]%ms%*[ \t]%ms", &name, &value) != 2)
		return conf_error (o, "enum requires name and value");

	ok = o->action->on_enum (o->cookie, name, value);
	free (name);
	free (value);
	return (ok && top) ? read_enum_conf (o, in) : ok;
}

static int read_tile_conf (struct chip_conf *o, FILE *in)
{
	char type[16];
	int ok = 1;

	while (ok && next_record (in) && fscanf (in, "%15s", type) == 1)
		ok = match (type, "arc:")     ? read_arrow   (o, in, 0) :
		     match (type, "word:")    ? read_word    (o, in, 0) :
		     match (type, "enum:")    ? read_enum    (o, in, 0) :
		     match (type, "unknown:") ? read_raw     (o, in, 0) :
		     conf_error (o, "unknown tile record type '%s'", type);

	return ok ? o->action->on_commit (o->cookie) : 0;
}

static int read_tile (struct chip_conf *o, FILE *in)
{
	char *name;
	int ok;

	if (fscanf (in, "%*[ \t]%ms", &name) != 1)
		return conf_error (o, "tile name required");

	ok = o->action->on_tile (o->cookie, name);
	free (name);
	return ok ? read_tile_conf (o, in) : 0;
}

static int read_tile_group (struct chip_conf *o, FILE *in)
{
	char *name;
	int ok;

	if (fscanf (in, "%*[ \t]%ms", &name) != 1)
		return conf_error (o, "tile name required");

	ok = o->action->on_tile (o->cookie, name);
	free (name);

	while (ok && fscanf (in, "%*[ \t]%ms", &name) == 1) {
		ok = o->action->on_tile (o->cookie, name);
		free (name);
	}

	return ok ? read_tile_conf (o, in) : 0;
}

static int read_bram (struct chip_conf *o, FILE *in)
{
	char *name;
	unsigned value;
	int ok;

	if (fscanf (in, "%*[ \t]%ms", &name) != 1)
		return conf_error (o, "bram name (index) required");

	ok = o->action->on_bram (o->cookie, name);
	free (name);

	while (ok && next_record (in)) {
		if (fscanf (in, "%x", &value) != 1)
			return conf_error (o, "hex bram value required");

		ok = o->action->on_bram_data (o->cookie, value);
	}

	return ok ? o->action->on_commit (o->cookie) : 0;
}

int read_conf (struct chip_conf *o, FILE *in)
{
	char verb[16];
	int ok = 1;

	while (ok && next_entry (in) && fscanf (in, "%15s", verb) == 1)
		ok = match (verb, ".device")      ? read_device     (o, in)    :
		     match (verb, ".comment")     ? read_comment    (o, in)    :
		     match (verb, ".sysconfig")   ? read_sysconfig  (o, in)    :
		     match (verb, ".unknown")     ? read_raw        (o, in, 1) :
		     match (verb, ".fixed_conn")  ? read_arrow      (o, in, 1) :
		     match (verb, ".mux")         ? read_mux        (o, in)    :
		     match (verb, ".config")      ? read_word       (o, in, 1) :
		     match (verb, ".config_enum") ? read_enum       (o, in, 1) :
		     match (verb, ".tile")        ? read_tile       (o, in)    :
		     match (verb, ".tile_group")  ? read_tile_group (o, in)    :
		     match (verb, ".bram_init")   ? read_bram       (o, in)    :
		     conf_error (o, "unknown verb '%s'", verb);

	return ferror (in) ? conf_error (o, "%s", strerror (errno)) : ok;
}
