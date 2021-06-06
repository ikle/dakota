/*
 * Trellis Chip Config
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef TRELLIS_CONF_H
#define TRELLIS_CONF_H  1

#include "chip-conf.h"

int trellis_read_conf (struct chip_conf *o, FILE *in);

#endif  /* TRELLIS_CONF_H */
