/*
 * Trellis Config Parser
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef TRELLIS_CONF_H
#define TRELLIS_CONF_H  1

#include <stdio.h>

struct config_action {
	int (*on_device)    (void *o, const char *name);
	int (*on_comment)   (void *o, const char *value);
	int (*on_sysconfig) (void *o, const char *name, const char *value);

	int (*on_tile)      (void *o, const char *name);

	int (*on_raw)       (void *o, unsigned bit);
	int (*on_arc)       (void *o, const char *sink, const char *source);
	int (*on_word)      (void *o, const char *name, const char *value);
	int (*on_enum)      (void *o, const char *name, const char *value);

	int (*on_bram)      (void *o, const char *name);
	int (*on_bram_data) (void *o, unsigned value);

	int (*on_commit)    (void *o);
};

struct config {
	const struct config_action *action;
	void *cookie;

	char error[256];
};

int read_conf (struct config *o, FILE *in);

#endif  /* TRELLIS_CONF_H */
