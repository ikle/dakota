/*
 * Dakota Model
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef DAKOTA_MODEL_H
#define DAKOTA_MODEL_H  1

#include <stdarg.h>
#include <stddef.h>

struct model *model_alloc (const char *name);
void model_free (struct model *o);

int model_error (struct model *o, const char *fmt, ...);
const char *model_status (struct model *o);

struct model *model_read (const char *path);

int model_add_input    (struct model *o, const char *name);
int model_add_output   (struct model *o, const char *name);
int model_add_wire     (struct model *o, const char *sink, const char *source);
int model_add_cell     (struct model *o, const char *type, const char *name);
int model_add_model    (struct model *o, const char *name);

const char *model_get_cell (struct model *o);

/*
 * model aoi22
 *	+ inputs A, B, C, D
 *	+ output Y
 *	+ cell and U0 + wire U0.A A    + wire U0.B B
 *	+ cell and U1 + wire U1.A C    + wire U1.B D
 *	+ cell nor U2 + wire U2.A U0.Y + wire U2.B U1.Y + wire Y U2.Y
 */

int model_add_tuple    (struct model *o, int size, ...);
int model_add_tuple_v  (struct model *o, int size, const char *argv[]);
int model_add_tuple_va (struct model *o, int size, va_list ap);
int model_add_param    (struct model *o, const char *name, const char *value);
int model_add_attr     (struct model *o, const char *name, const char *value);

int model_commit       (struct model *o);

#endif  /* DAKOTA_MODEL_H */
