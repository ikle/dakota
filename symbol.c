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

#include <dakota/symbol.h>

enum node_type {
	NODE_MOVE,
	NODE_LINE,
	NODE_ARC,
	NODE_MARK,
	NODE_TEXT,
};

struct node {
	struct node *next;
	enum node_type type;
	int x, y;
	union {
		int degree;
		char *mark;
		char *text;
	};
};

static struct node *node_alloc (enum node_type type, int x, int y)
{
	struct node *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	o->next = NULL;
	o->type = type;
	o->x    = x;
	o->y    = y;
	return o;
}

static void node_free_one (struct node *o)
{
	if (o == NULL)
		return;

	switch (o->type) {
	case NODE_MARK:
		free (o->mark);
		break;
	case NODE_TEXT:
		free (o->text);
		break;
	default:
		break;
	}

	free (o);
}

static void node_free (struct node *o)
{
	struct node *next;

	for (; o != NULL; o = next) {
		next = o->next;
		node_free_one (o);
	}
}

struct symbol {
	struct node *head, *last;
};

struct symbol *symbol_alloc (void)
{
	struct symbol *o;

	if ((o = malloc (sizeof (*o))) == NULL)
		return NULL;

	o->last = o->head = NULL;
	return o;
}

void symbol_free (struct symbol *o)
{
	if (o == NULL)
		return;

	node_free (o->head);
	free (o);
}

static struct node *
symbol_add_node (struct symbol *o, enum node_type type, int x, int y)
{
	struct node *s;

	if ((s = node_alloc (type, x, y)) == NULL)
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

	if (s != NULL && s->type == NODE_MOVE) {
		s->x = x;
		s->y = y;
		return 1;
	}

	if ((s = symbol_add_node (o, NODE_MOVE, x, y)) == NULL)
		return 0;

	return 1;
}

int symbol_line (struct symbol *o, int x, int y)
{
	struct node *s;

	if ((s = symbol_add_node (o, NODE_LINE, x, y)) == NULL)
		return 0;

	return 1;
}

int symbol_arc (struct symbol *o, int x, int y, int degree)
{
	struct node *s;

	if ((s = symbol_add_node (o, NODE_ARC, x, y)) == NULL)
		return 0;

	s->degree = degree;
	return 1;
}

int symbol_mark (struct symbol *o, int x, int y, const char *mark)
{
	struct node *last = o->last;
	struct node *s;

	if ((s = symbol_add_node (o, NODE_MARK, x, y)) == NULL)
		return 0;

	if ((s->mark = strdup (mark)) == NULL)
		goto no_mark;

	return 1;
no_mark:
	symbol_drop_tail (o, last);
	return 0;
}

int symbol_text (struct symbol *o, int x, int y, const char *text)
{
	struct node *last = o->last;
	struct node *s;

	if ((s = symbol_add_node (o, NODE_TEXT, x, y)) == NULL)
		return 0;

	if ((s->text = strdup (text)) == NULL)
		goto no_text;

	return 1;
no_text:
	symbol_drop_tail (o, last);
	return 0;
}

int symbol_blit (struct symbol *o, int x, int y, const struct symbol *tile)
{
	struct node *last = o->last;
	const struct node *s;
	int ok = 1;

	for (s = tile->head; s != NULL; s = s->next)
		switch (s->type) {
		case NODE_MOVE:
			ok &= symbol_move (o, x + s->x, y + s->y);
			break;
		case NODE_LINE:
			ok &= symbol_line (o, x + s->x, y + s->y);
			break;
		case NODE_ARC:
			ok &= symbol_arc  (o, x + s->x, y + s->y, s->degree);
			break;
		case NODE_MARK:
			ok &= symbol_mark (o, x + s->x, y + s->y, s->mark);
			break;
		case NODE_TEXT:
			ok &= symbol_text (o, x + s->x, y + s->y, s->text);
			break;
		}

	if (!ok)
		symbol_drop_tail (o, last);

	return ok;
}
