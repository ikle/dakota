/*
 * Dakota Symbol Parser
 *
 * Note, it is NOT generic graphics library.
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include <dakota/symbol.h>
#include <dakota/util.h>

static int on_move (struct symbol *o, const struct shell_cmd *cmd)
{
	int x, y;

	if (cmd->argc != 3)
		return 0;

	x = atoi (cmd->argv[1]);
	y = atoi (cmd->argv[2]);

	return symbol_move (o, x, y);
}

static int on_line (struct symbol *o, const struct shell_cmd *cmd)
{
	int x, y;

	if (cmd->argc != 3)
		return 0;

	x = atoi (cmd->argv[1]);
	y = atoi (cmd->argv[2]);

	return symbol_line (o, x, y);
}

static int on_arc (struct symbol *o, const struct shell_cmd *cmd)
{
	int x, y, degree;

	if (cmd->argc != 4)
		return 0;

	x      = atoi (cmd->argv[1]);
	y      = atoi (cmd->argv[2]);
	degree = atoi (cmd->argv[3]);

	return symbol_arc (o, x, y, degree);
}

static int on_mark (struct symbol *o, const struct shell_cmd *cmd)
{
	int x, y;

	if (cmd->argc != 4)
		return 0;

	x = atoi (cmd->argv[1]);
	y = atoi (cmd->argv[2]);

	return symbol_mark (o, x, y, cmd->argv[3]);
}

static int on_text (struct symbol *o, const struct shell_cmd *cmd)
{
	int x, y;

	if (cmd->argc != 5)
		return 0;

	x = atoi (cmd->argv[1]);
	y = atoi (cmd->argv[2]);

	return symbol_text (o, x, y, cmd->argv[3][0], cmd->argv[4]);
}

#define PROC(name, func) \
	strcmp (cmd->argv[0], "." #name)  == 0 ? on_ ## func (o, cmd)

struct symbol *symbol_read (const char *path)
{
	struct symbol *o;
	struct shell *sh;
	const struct shell_cmd *cmd;
	int ok = 1;

	if ((o = symbol_alloc ()) == NULL)
		return NULL;

	if ((sh = dakota_open ("symbol", path)) == NULL)
		goto no_shell;

	while ((cmd = shell_next (sh)) != NULL) {
		ok = PROC (move, move) :
		     PROC (line, line) :
		     PROC (arc,  arc)  :
		     PROC (mark, mark) :
		     PROC (text, text) :
		     0;

		if (!ok)
			break;
	}

	shell_free (sh);

	if (!ok)
		goto no_parse;

	return o;
no_parse:
no_shell:
	symbol_free (o);
	return NULL;
}
