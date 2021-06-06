/*
 * Trellis Config Parser Test
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>

#include <err.h>

#include "chip-conf-show.h"
#include "trellis-conf.h"

int main (int argc, char *argv[])
{
	struct chip_conf *c;
	FILE *in;
	int ok;

	if (argc != 2)
		errx (0, "\n\ttrellis-test <conf-file>");

	if ((c = chip_conf_show_alloc ()) == NULL)
		err (1, "cannot allocate config actor");

	if ((in = fopen (argv[1], "r")) == NULL)
		err (1, "cannot open conf file %s", argv[1]);

	ok = trellis_read_conf (c, in);
	fclose (in);

	if (!ok)
		errx (1, c->error);

	chip_conf_show_free (c);
	return 0;
}
