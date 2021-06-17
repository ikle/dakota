/*
 * Dakota Model Parser Test
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <err.h>

#include <dakota/model.h>

int main (int argc, char *argv[])
{
	struct model *o;

	if (argc != 2)
		errx (1, "\n\tmodel-read-test <input-file>");

	if ((o = model_read (argv[1])) == NULL)
		err (1, "cannot create model from %s", argv[1]);

	if (model_status (o) != NULL)
		warnx ("%s", model_status (o));

	model_free (o);
	return 0;
}
