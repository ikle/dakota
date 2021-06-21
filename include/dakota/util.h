/*
 * Dakota Common Helpers
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_UTIL_H
#define DAKOTA_UTIL_H

#include <dakota/shell.h>

struct shell *dakota_open (const char *category, const char *path);

#endif  /* DAKOTA_UTIL_H */
