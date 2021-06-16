/*
 * Dakota Shell Parser
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_SHELL_H
#define DAKOTA_SHELL_H  1

#include <stddef.h>

struct shell *shell_alloc (const char *fmt, ...);
void shell_free (struct shell *o);

struct shell_cmd {
	size_t argc;
	const char **argv;
};

const struct shell_cmd *shell_next (struct shell *o);

#endif  /* DAKOTA_SHELL_H */
