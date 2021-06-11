/*
 * Dakota Shell Parser
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <dakota/shell.h>
#include <dakota/string.h>

struct shell {
	FILE *in;
	size_t nchars, last;
	char *line;
	size_t nwords;
	struct shell_cmd cmd;
};

struct shell *shell_alloc (const char *fmt, ...)
{
	struct shell *o;
	va_list ap;
	char *path;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	va_start(ap, fmt);
	path = make_string_va (fmt, ap);
	va_end(ap);

	if ((o->in = fopen (path, "r")) == NULL)
		goto no_file;

	free (path);

	o->nchars     = 0;
	o->line       = NULL;
	o->nwords     = 0;
	o->cmd.argv   = NULL;
	return o;
no_file:
	free (path);
	free (o);
	return NULL;
}

void shell_free (struct shell *o)
{
	if (o == NULL)
		return;

	free (o->line);
	free (o->cmd.argv);
	free (o);
}

static void debug (const char *fmt, ...)
{
#ifndef DEBUG
	/* do nothing */
#else
	va_list ap;

	fputs ("D: ", stderr);

	va_start(ap, fmt);
	vfprintf (stderr, fmt, ap);
	va_end(ap);

	fputc ('\n', stderr);
#endif
}

static size_t get_next_size (size_t min, size_t have, size_t request)
{
	size_t size = (have == 0 ? min: have), next;

	for (; size < request; size = next) {
		next = size * 2;

		if (next < size)
			return SIZE_MAX;
	}

	return size;
}

static int resize_line (struct shell *o, size_t size)
{
	size_t next, i;
	char *p;

	if (size <= o->nchars)
		return 1;

	next = get_next_size (128, o->nchars, size);

	if ((p = realloc (o->line, next)) == NULL)
		return 0;

	for (i = 0; i < o->cmd.argc; ++i)
		o->cmd.argv[i] += p - o->line;

	debug ("next max line size = %zu", next);

	o->nchars = next;
	o->line   = p;
	return 1;
}

static int resize_words (struct shell *o, size_t count)
{
	size_t next;
	char **p;

	if (count <= o->nwords)
		return 1;

	next = get_next_size (8, o->nwords, count);

	if ((p = realloc (o->cmd.argv, next * sizeof (o->cmd.argv[0]))) == NULL)
		return 0;

	debug ("next max words count = %zu", next);

	o->nwords   = next;
	o->cmd.argv = p;
	return 1;
}

static int push_word (struct shell *o)
{
	if (!resize_words (o, o->cmd.argc + 1))
		return 0;

	o->cmd.argv[o->cmd.argc++] = o->line + o->last;
	return 1;
}

static int push_char (struct shell *o, int a)
{
	if (!resize_line (o, o->last + 1))
		return 0;

	o->line[o->last++] = a;
	return 1;
}

static size_t get_word (struct shell *o)
{
	int a;
start:
	switch (a = fgetc (o->in)) {
	case EOF:
	case '\n':	goto end;
	case '\t':
	case ' ':	goto start;
	case '#':	goto comment;
	default:	goto head;
	}
comment:
	switch (a = fgetc (o->in)) {
	case EOF:
	case '\n':	goto end;
	default:	goto comment;
	}
head:
	if (!push_word (o))
		return 0;
word:
	if (!push_char (o, a))
		return 0;

	switch (a = fgetc (o->in)) {
	case EOF:
	case '\t':
	case '\n':
	case ' ':	goto tail;
	default:	goto word;
	}
tail:
	if (!push_char (o, '\0'))
		return 0;

	debug ("got word %s", o->cmd.argv[o->cmd.argc - 1]);

	if (a != '\n')
		goto start;
end:
	if (o->cmd.argc > 0) {
		debug ("got %zu words", o->cmd.argc);
		return o->cmd.argc;
	}

	if (a == EOF) {
		debug ("end of file");
		return 0;
	}

	debug ("skip empty line");
	goto start;
}

const struct shell_cmd *shell_next (struct shell *o)
{
	o->last     = 0;
	o->cmd.argc = 0;

	if (get_word (o) == 0)
		return NULL;

	return &o->cmd;
}
