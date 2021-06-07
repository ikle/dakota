/*
 * Chip Config
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef CHIP_CONF_H
#define CHIP_CONF_H  1

#include <stdio.h>

struct chip_action {
	int (*on_device)    (void *o, const char *name);
	int (*on_comment)   (void *o, const char *value);
	int (*on_sysconfig) (void *o, const char *name, const char *value);

	int (*on_tile)      (void *o, const char *name);

	int (*on_raw)       (void *o, unsigned bit);
	int (*on_arrow)     (void *o, const char *sink, const char *source);

	int (*on_mux)       (void *o, const char *name);
	int (*on_mux_data)  (void *o, const char *source, unsigned *bits);

	int (*on_word)      (void *o, const char *name, const char *value);
	int (*on_word_data) (void *o, unsigned *bits);

	int (*on_enum)      (void *o, const char *name, const char *value);
	int (*on_enum_data) (void *o, const char *value, unsigned *bits);

	int (*on_bram)      (void *o, const char *name);
	int (*on_bram_data) (void *o, unsigned value);

	int (*on_commit)    (void *o);
};

struct chip_conf {
	const struct chip_action *action;
	void *cookie;

	char error[256];
};

int chip_error (struct chip_conf *o, const char *fmt, ...);

#endif  /* CHIP_CONF_H */
