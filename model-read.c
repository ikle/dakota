/*
 * Dakota Model Parser
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dakota/model.h>
#include <dakota/shell.h>
#include <dakota/string.h>

static int on_model (struct model *o, const struct shell_cmd *cmd)
{
	if (cmd->argc < 2)
		return model_error (o, "no model name given");

	return model_add_model (o, cmd->argv[1]);
}

static int on_inputs (struct model *o, const struct shell_cmd *cmd)
{
	size_t i;

	for (i = 1; i < cmd->argc; ++i)
		if (!model_add_input (o, cmd->argv[i]))
			return 0;

	return 1;
}

static int on_outputs (struct model *o, const struct shell_cmd *cmd)
{
	size_t i;

	for (i = 1; i < cmd->argc; ++i)
		if (!model_add_output (o, cmd->argv[i]))
			return 0;

	return 1;
}

static int on_cell (struct model *o, const struct shell_cmd *cmd)
{
	size_t i;

	if (cmd->argc < 2)
		return model_error (o, "no cell type given");

	if (!model_add_cell (o, cmd->argv[1], NULL))
		return 0;

	if (!model_add_attr (o, "cell-kind", cmd->argv[0] + 1))
		return 0;

	for (i = 2; i < cmd->argc; ++i)
		if (!model_add_attr (o, "cell-bind", cmd->argv[i]))
			return 0;

	return 1;
}

static int on_table (struct model *o, const struct shell_cmd *cmd)
{
	size_t i;
	char count[22];
	int ok = 1;

	if (cmd->argc < 2)
		return model_error (o, "empty table");

	snprintf (count, sizeof (count), "%zu", cmd->argc - 2);

	ok &= model_add_cell (o, "table", NULL);
	ok &= model_add_attr (o, "cell-kind", cmd->argv[0] + 1);
	ok &= model_add_attr (o, "cell-inputs", count);
	ok &= model_add_attr (o, "cell-outputs", "1");

	for (i = 1; i < cmd->argc; ++i)
		ok &= model_add_attr (o, "cell-bind", cmd->argv[i]);

	return ok;
}

static int on_latch (struct model *o, const struct shell_cmd *cmd)
{
	int ok = 1;

	if (cmd->argc < 3)
		return model_error (o, "no latch input and output given");

	ok &= model_add_cell (o, "latch", NULL);
	ok &= model_add_attr (o, "cell-kind", "latch");

	if (cmd->argc >= 4)
		ok &= model_add_attr (o, "cell-edge", cmd->argv[3]);

	if (cmd->argc >= 5 && strcmp (cmd->argv[4], "NIL") != 0) {
		ok &= model_add_attr (o, "cell-inputs",  "2");
		ok &= model_add_attr (o, "cell-bind", cmd->argv[4]);
	}
	else
		ok &= model_add_attr (o, "cell-inputs",  "1");

	ok &= model_add_attr (o, "cell-bind", cmd->argv[1]);
	ok &= model_add_attr (o, "cell-outputs", "1");
	ok &= model_add_attr (o, "cell-bind", cmd->argv[2]);

	if (cmd->argc >= 6)
		ok &= model_add_attr (o, "cell-init", cmd->argv[5]);

	return ok;
}

static int on_tuple (struct model *o, const struct shell_cmd *cmd)
{
	return model_add_tuple_v (o, cmd->argc, cmd->argv);
}

static int on_cname (struct model *o, const struct shell_cmd *cmd)
{
	if (cmd->argc < 2)
		return model_error (o, "no common name given");

	return model_add_attr (o, "cname", cmd->argv[1]);
}

static int on_param (struct model *o, const struct shell_cmd *cmd)
{
	const char *value;

	if (cmd->argc < 2)
		return model_error (o, "no parameter name given");

	value = cmd->argc < 3 ? NULL : cmd->argv[2];

	return model_add_param (o, cmd->argv[1], value);
}

static int on_attr (struct model *o, const struct shell_cmd *cmd)
{
	const char *value;

	if (cmd->argc < 2)
		return model_error (o, "no attribute name given");

	value = cmd->argc < 3 ? NULL : cmd->argv[2];

	return model_add_attr (o, cmd->argv[1], value);
}

static struct model *on_model_root (const struct shell_cmd *cmd)
{
	const char *name = (cmd->argc < 2) ? "empty" : cmd->argv[1];
	struct model *o;

	if ((o = model_alloc (name)) == NULL)
		return NULL;

	if (cmd->argc < 2)
		model_error (o, "no model name given");

	return o;
}

struct model *model_read (const char *path)
{
	struct model *o = NULL;
	struct shell *sh;
	const struct shell_cmd *cmd;
	int ok;

	if ((sh = shell_alloc ("%s", path)) == NULL)
		return NULL;

	while ((cmd = shell_next (sh)) != NULL) {
		if (o == NULL) {
			if (strcmp (cmd->argv[0], ".model") != 0)
				continue;  /* ignore all until model is given */

			if ((o = on_model_root (cmd)) == NULL ||
			    model_status (o) != NULL)
				break;

			continue;
		}

#define PROC(name, func) \
	strcmp (cmd->argv[0], "." #name)  == 0 ? on_ ## func (o, cmd)

		ok = PROC (inputs,  inputs)  :
		     PROC (outputs, outputs) :
		     PROC (gate,    cell)    :
		     PROC (subckt,  cell)    :
		     PROC (names,   table)   :
		     PROC (latch,   latch)   :
		     PROC (model,   model)   :
		     PROC (cname,   cname)   :
		     PROC (param,   param)   :
		     PROC (attr,    attr)    :
		     cmd->argv[0][0] != '.' ? on_tuple (o, cmd) : 1;

		if (!ok)
			break;
	}

	if (model_status (o) == NULL)
		model_commit (o);

	shell_free (sh);
	return o;
}
