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

#include <dakota/shell.h>
#include <dakota/symbol.h>

static
int on_move (struct shell *sh, struct symbol *o, const struct shell_cmd *cmd)
{
	int x, y;

	if (cmd->argc != 3)
		return 0;

	x = atoi (cmd->argv[1]);
	y = atoi (cmd->argv[2]);

	return symbol_move (o, x, y);
}

static
int on_line (struct shell *sh, struct symbol *o, const struct shell_cmd *cmd)
{
	int x, y;

	if (cmd->argc != 3)
		return 0;

	x = atoi (cmd->argv[1]);
	y = atoi (cmd->argv[2]);

	return symbol_line (o, x, y);
}

static
int on_arc (struct shell *sh, struct symbol *o, const struct shell_cmd *cmd)
{
	int x, y, degree;

	if (cmd->argc != 4)
		return 0;

	x      = atoi (cmd->argv[1]);
	y      = atoi (cmd->argv[2]);
	degree = atoi (cmd->argv[3]);

	return symbol_arc (o, x, y, degree);
}

static
int on_mark (struct shell *sh, struct symbol *o, const struct shell_cmd *cmd)
{
	int x, y;

	if (cmd->argc != 4)
		return 0;

	x = atoi (cmd->argv[1]);
	y = atoi (cmd->argv[2]);

	return symbol_mark (o, x, y, cmd->argv[3]);
}

static
int on_text (struct shell *sh, struct symbol *o, const struct shell_cmd *cmd)
{
	int x, y;

	if (cmd->argc != 5)
		return 0;

	x = atoi (cmd->argv[1]);
	y = atoi (cmd->argv[2]);

	return symbol_text (o, x, y, cmd->argv[3][0], cmd->argv[4]);
}

static
int on_blit (struct shell *sh, struct symbol *o, const struct shell_cmd *cmd)
{
	int x, y, dir;

	if (cmd->argc != 5)
		return 0;

	x   = atoi (cmd->argv[1]);
	y   = atoi (cmd->argv[2]);
	dir = atoi (cmd->argv[3]);

	return symbol_blit (o, x, y, dir, cmd->argv[4]);
}

static struct symbol *
symbol_parse (struct shell *sh, struct symbol *parent, const char *name);

static
int on_tile (struct shell *sh, struct symbol *o, const struct shell_cmd *cmd)
{
	struct symbol *tile;

	if (cmd->argc != 2)
		return 0;

	if ((tile = symbol_parse (sh, o, cmd->argv[1])) == NULL)
		return 0;

	if (symbol_add_tile (o, tile))
		return 1;

	symbol_free (tile);
	return 0;
}

static int is_end (const struct shell_cmd *cmd)
{
	return cmd->argc == 1 && strcmp (cmd->argv[0], "end") == 0;
}

#define PROC(name, func) \
	strcmp (cmd->argv[0], #name)  == 0 ? on_ ## func (sh, o, cmd)

static struct symbol *
symbol_parse (struct shell *sh, struct symbol *parent, const char *name)
{
	struct symbol *o;
	const struct shell_cmd *cmd;
	int ok = 1;

	if ((o = symbol_alloc (parent, name)) == NULL)
		return NULL;

	while (ok && (cmd = shell_next (sh)) != NULL && !is_end (cmd))
		ok = PROC (move, move) :
		     PROC (line, line) :
		     PROC (arc,  arc)  :
		     PROC (mark, mark) :
		     PROC (text, text) :
		     PROC (blit, blit) :
		     PROC (tile, tile) :
		     0;

	if (ok)
		return o;

	symbol_free (o);
	return NULL;
}

struct symbol *symbol_read (const char *name, const char *path)
{
	struct shell *sh;
	struct symbol *o;

	if ((sh = shell_alloc ("symbol", path)) == NULL)
		return NULL;

	o = symbol_parse (sh, NULL, name);
	shell_free (sh);
	return o;
}
