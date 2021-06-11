/*
 * Dakota Shell Parser Test
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <err.h>
#include <stdio.h>

#include <dakota/shell.h>

int main (int argc, char *argv[])
{
	struct shell *o;
	const struct shell_cmd *cmd;
	size_t i;

	if (argc != 2)
		errx (1, "\n\tshell-test <input-file>");

	if ((o = shell_alloc ("%s", argv[1])) == NULL)
		err (1, "cannot open %s", argv[1]);

	while ((cmd = shell_next (o)) != NULL) {
		printf ("got %zu words:", cmd->argc);

		for (i = 0; i < cmd->argc; ++i)
			printf (" %s", cmd->argv[i]);

		printf ("\n");
	}

	shell_free (o);
	return 0;
}
