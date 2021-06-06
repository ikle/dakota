/*
 * Chip Config Show (Debug) Action
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef CHIP_CONF_SHOW_H
#define CHIP_CONF_SHOW_H  1

#include "chip-conf.h"

struct chip_conf *chip_conf_show_alloc (void);
void chip_conf_show_free (struct chip_conf *o);

#endif  /* CHIP_CONF_SHOW_H */
