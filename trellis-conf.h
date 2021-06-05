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
	int (*on_device)    (void *cookie, const char *name);
	int (*on_comment)   (void *cookie, const char *value);
	int (*on_sysconfig) (void *cookie, const char *name, const char *value);

	int (*on_tile)    (void *cookie, const char *name);
	int (*on_arc)     (void *cookie, const char *sink, const char *source);
	int (*on_word)    (void *cookie, const char *name, const char *value);
	int (*on_enum)    (void *cookie, const char *name, const char *value);
	int (*on_unknown) (void *cookie, const char *value);

	int (*on_bram) (void *cookie, unsigned index);
	int (*on_data) (void *cookie, unsigned bram, size_t i, unsigned value);

	int (*on_commit) (void *cookie);
};

struct config {
	const struct config_action *action;
	void *cookie;

	char error[256];
};

int read_conf (struct config *o, FILE *in);

#endif  /* TRELLIS_CONF_H */
