/*
 * Dakota Chip Cache
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_CACHE_H
#define DAKOTA_CACHE_H  1

#include <cmdb.h>

struct cmdb *
dakota_open_grid (const char *family, const char *device, const char *mode);

#endif  /* DAKOTA_CACHE_H */
