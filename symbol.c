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
		struct {
			int dir;
			char name[1];
		} blit;
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

int symbol_blit (struct symbol *o, int x, int y, int dir, const char *name)
{
	struct node *s;
	const size_t extra = sizeof (s->blit) + strlen (name);

	if ((s = symbol_add_node (o, SYMBOL_BLIT, x, y, extra)) == NULL)
		return 0;

	s->blit.dir = dir;
	strcpy (s->blit.name, name);
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
		case SYMBOL_BLIT:
			ok = fn (cookie, s->type, s->x, s->y, s->blit.dir,
				 s->blit.name);
			break;
		}

	for (i = 0; ok && i < o->ntiles; ++i)
		ok = fn (cookie, SYMBOL_TILE, 0, 0, o->tile[i]->name)	&&
		     symbol_walk (o->tile[i], fn, cookie)		&&
		     fn (cookie, SYMBOL_END, 0, 0);

	return ok;
}
