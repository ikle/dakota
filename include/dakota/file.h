/*
 * Dakota File Helpers
 *
 * Copyright (c) 2021-2022 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_FILE_H
#define DAKOTA_FILE_H

#include <stdio.h>

FILE *dakota_open (const char *category, const char *path);

#endif  /* DAKOTA_FILE_H */
