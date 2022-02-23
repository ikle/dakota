/*
 * Dakota Shell Parser
 *
 * Copyright (c) 2021-2022 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dakota/file.h>
#include <dakota/shell.h>

struct shell {
	FILE *in;
	size_t nchars, last;
	char *line;
	size_t nwords;
	struct shell_cmd cmd;
};

struct shell *shell_alloc (const char *category, const char *path)
{
	struct shell *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	if (strcmp (path, "-") == 0)
		o->in = stdin;
	else
	if ((o->in = dakota_open (category, path)) == NULL)
		goto no_file;

	o->nchars     = 0;
	o->line       = NULL;
	o->nwords     = 0;
	o->cmd.argv   = NULL;
	o->cmd.lineno = 0;
	return o;
no_file:
	free (o);
	return NULL;
}

void shell_free (struct shell *o)
{
	if (o == NULL)
		return;

	fclose (o->in);
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
	size_t size = (have == 0 ? min : have), next;

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

static int get_word_char (struct shell *o)
{
	int a;

	for (;;) {
		if ((a = fgetc (o->in)) == '#') {
			while ((a = fgetc (o->in)) != EOF && a != '\n') {}

			return a;
		}

		if (a != '\\')
			return a;

		if ((a = fgetc (o->in)) != '\n') {
			ungetc (a, o->in);
			return '\\';
		}

		++o->cmd.lineno;
		/* collapse backslash + newline, concatenate lines */
	}

	return a;  /* unreachable */
}

static size_t get_word (struct shell *o)
{
	int a;
l_start:
	for (o->cmd.indent = 0; (a = fgetc (o->in)) != EOF; )
		if (a == ' ')
			++o->cmd.indent;
		else
		if (a == '\t')
			o->cmd.indent = (o->cmd.indent + 8) & ~7;
		else {
			ungetc (a, o->in);
			break;
		}
w_start:
	switch (a = get_word_char (o)) {
	case EOF:
	case '\n':	goto end;
	case '\t':
	case ' ':	goto w_start;
	case '"':	goto string;
	default:	goto w_head;
	}
string:
	if (!push_word (o))
		return 0;

	goto s_first;
s_next:
	if (!push_char (o, a))
		return 0;
s_first:
	switch (a = fgetc (o->in)) {
	case EOF:
	case '"':	goto tail;
	case '\\':	goto s_escape;
	default:	goto s_next;
	}
s_escape:
	switch (a = fgetc (o->in)) {
	case EOF:	goto tail;
	default:	goto s_next;
	}
w_head:
	if (!push_word (o))
		return 0;
w_next:
	if (!push_char (o, a))
		return 0;

	switch (a = get_word_char (o)) {
	case EOF:
	case '\t':
	case '\n':
	case ' ':	goto tail;
	default:	goto w_next;
	}
tail:
	if (!push_char (o, '\0'))
		return 0;

	debug ("got word %s", o->cmd.argv[o->cmd.argc - 1]);

	if (a != '\n')
		goto w_start;
end:
	++o->cmd.lineno;

	if (o->cmd.argc > 0) {
		debug ("got %zu words", o->cmd.argc);
		return o->cmd.argc;
	}

	if (a == EOF) {
		debug ("end of file");
		return 0;
	}

	debug ("skip empty line");
	goto l_start;
}

const struct shell_cmd *shell_next (struct shell *o)
{
	o->last     = 0;
	o->cmd.argc = 0;

	if (get_word (o) == 0)
		return NULL;

	return &o->cmd;
}
