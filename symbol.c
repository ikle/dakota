/*
 * Dakota Symbol
 *
 * Note, it is NOT generic graphics library.
 *
 * Copyright (c) 2021 Alexei A. Smekalkine <ikle@ikle.ru>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>
#include <string.h>

#include <dakota/array.h>
#include <dakota/symbol.h>

struct node {
	struct node *next;
	int type, x, y;
	union {
		struct {
			int angle;
		} arc;
		char mark[1];
		struct {
			int dir;
			char string[1];
		} text;
	};
};

static struct node *node_alloc (int type, int x, int y, size_t extra)
{
	struct node *o;

	if ((o = malloc (offsetof (struct node, arc) + extra)) == NULL)
		return NULL;

	o->next = NULL;
	o->type = type;
	o->x    = x;
	o->y    = y;
	return o;
}

static void node_free (struct node *o)
{
	struct node *next;

	for (; o != NULL; o = next) {
		next = o->next;
		free (o);
	}
}

struct symbol {
	struct symbol *parent;
	char *name;
	struct node *head, *last;

	size_t ntiles;
	struct symbol **tile;
};

struct symbol *symbol_alloc (struct symbol *parent, const char *name)
{
	struct symbol *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	o->parent = parent;

	if ((o->name = strdup (name)) == NULL)
		goto no_name;

	o->last = o->head = NULL;
	o->ntiles = 0;
	o->tile   = NULL;
	return o;
no_name:
	free (o);
	return NULL;
}

void symbol_free (struct symbol *o)
{
	if (o == NULL)
		return;

	array_free (o->tile, o->ntiles, symbol_free);
	node_free (o->head);
	free (o->name);
	free (o);
}

static struct node *
symbol_add_node (struct symbol *o, int type, int x, int y, size_t extra)
{
	struct node *s;

	if ((s = node_alloc (type, x, y, extra)) == NULL)
		return NULL;

	if (o->head == NULL)
		o->head = s;
	else
		o->last->next = s;

	o->last = s;
	return s;
}

static void symbol_drop_tail (struct symbol *o, struct node *last)
{
	if (last == NULL) {
		node_free (o->head);
		o->last = o->head = NULL;
		return;
	}

	node_free (last->next);
	last->next = NULL;
	o->last = last;
}

int symbol_move (struct symbol *o, int x, int y)
{
	struct node *s = o->last;

	if (s != NULL && s->type == SYMBOL_MOVE) {
		s->x = x;
		s->y = y;
		return 1;
	}

	if ((s = symbol_add_node (o, SYMBOL_MOVE, x, y, 0)) == NULL)
		return 0;

	return 1;
}

int symbol_line (struct symbol *o, int x, int y)
{
	struct node *s;

	if ((s = symbol_add_node (o, SYMBOL_LINE, x, y, 0)) == NULL)
		return 0;

	return 1;
}

int symbol_arc (struct symbol *o, int x, int y, int angle)
{
	struct node *s;
	const size_t extra = sizeof (s->arc);

	if ((s = symbol_add_node (o, SYMBOL_ARC, x, y, extra)) == NULL)
		return 0;

	s->arc.angle = angle;
	return 1;
}

int symbol_mark (struct symbol *o, int x, int y, const char *mark)
{
	struct node *s;
	const size_t extra = sizeof (s->mark) + strlen (mark);

	if ((s = symbol_add_node (o, SYMBOL_MARK, x, y, extra)) == NULL)
		return 0;

	strcpy (s->mark, mark);
	return 1;
}

int symbol_text (struct symbol *o, int x, int y, int dir, const char *text)
{
	struct node *s;
	const size_t extra = sizeof (s->text) + strlen (text);

	if ((s = symbol_add_node (o, SYMBOL_TEXT, x, y, extra)) == NULL)
		return 0;

	s->text.dir = dir;
	strcpy (s->text.string, text);
	return 1;
}

int symbol_add_tile (struct symbol *o, struct symbol *tile)
{
	const size_t ntiles = o->ntiles + 1;
	struct symbol **p;

	if ((p = array_resize (o->tile, ntiles)) == NULL)
		return 0;

	tile->parent = o;
	p[o->ntiles] = tile;

	o->tile   = p;
	o->ntiles = ntiles;
	return 1;
}

static void vector_rotate (int x, int y, int dir, int *nx, int *ny)
{
	switch (dir) {
	default:	*nx =  x; *ny =  y; return;
	case 'N':	*nx = -y; *ny =  x; return;
	case 'W':	*nx = -x; *ny = -y; return;
	case 'S':	*nx =  y; *ny = -x; return;
	}
}

static int dir_index (int dir)
{
	switch (dir) {
	default:	return 0;
	case 'N':	return 1;
	case 'W':	return 2;
	case 'S':	return 3;
	}
}

static int dir_rotate (int dira, int dirb)
{
	char ring[4] = "ONWS";

	return ring[(dir_index (dira) + dir_index (dirb)) & 3];
}

int symbol_blit (struct symbol *o, int x, int y, int dir,
		 const struct symbol *tile)
{
	struct node *last = o->last;
	const struct node *s;
	int nx, ny, ndir, ok = 1;

	for (s = tile->head; s != NULL; s = s->next) {
		vector_rotate (s->x, s->y, dir, &nx, &ny);

		switch (s->type) {
		case SYMBOL_MOVE:
			ok &= symbol_move (o, x + nx, y + ny);
			break;
		case SYMBOL_LINE:
			ok &= symbol_line (o, x + nx, y + ny);
			break;
		case SYMBOL_ARC:
			ok &= symbol_arc  (o, x + nx, y + ny, s->arc.angle);
			break;
		case SYMBOL_MARK:
			ok &= symbol_mark (o, x + nx, y + ny, s->mark);
			break;
		case SYMBOL_TEXT:
			ndir = dir_rotate (s->text.dir, dir);
			ok &= symbol_text (o, x + nx, y + ny,
					   ndir, s->text.string);
			break;
		}
	}

	if (!ok)
		symbol_drop_tail (o, last);

	return ok;
}

int symbol_walk (const struct symbol *o, symbol_fn *fn, void *cookie)
{
	const struct node *s;
	int ok = 1;
	size_t i;

	for (s = o->head; ok && s != NULL; s = s->next)
		switch (s->type) {
		case SYMBOL_MOVE:
			ok = fn (cookie, s->type, s->x, s->y);
			break;
		case SYMBOL_LINE:
			ok = fn (cookie, s->type, s->x, s->y);
			break;
		case SYMBOL_ARC:
			ok = fn (cookie, s->type, s->x, s->y, s->arc.angle);
			break;
		case SYMBOL_MARK:
			ok = fn (cookie, s->type, s->x, s->y, s->mark);
			break;
		case SYMBOL_TEXT:
			ok = fn (cookie, s->type, s->x, s->y, s->text.dir,
				 s->text.string);
			break;
		}

	for (i = 0; ok && i < o->ntiles; ++i)
		ok = fn (cookie, SYMBOL_TILE, 0, 0, o->tile[i]->name)	&&
		     symbol_walk (o->tile[i], fn, cookie)		&&
		     fn (cookie, SYMBOL_END, 0, 0);

	return ok;
}
