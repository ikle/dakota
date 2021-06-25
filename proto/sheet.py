#!/usr/bin/python3

import cairo

def font_size (c, size, right = True):
	xx = size
	yy = -size if right else size

	c.set_font_matrix (cairo.Matrix (xx, 0, 0, yy, 0, 0))

def create (M, W, H, S = 20, step = 1, right = True, grid = True):
	SW, SH = W * S, H * S
	IW, IH = SW + M * 2 + 1, SH + M * 2 + 1

	surface = cairo.ImageSurface (cairo.FORMAT_ARGB32, IW, IH)
	# surface = cairo.SVGSurface ("cairo.svg", W, H)

	c = cairo.Context (surface)

	if right:
		# use right-handed Cartesian coordinate system
		c.scale (1.0, -1.0)
		c.translate (0, -IH)

	# white background
	c.set_source_rgb (1.0, 1.0, 1.0)
	c.rectangle (0, 0, IW, IH)
	c.fill ()

	# clip out margins
	c.rectangle (M, M, SW + 1, SH + 1)
	c.clip ()

	# coordinates in center of pixels in user area
	c.translate (M + 0.5, M + 0.5)

	c.set_line_cap  (1)  # cairo.LineCap.ROUND
	c.set_line_join (1)  # cairo.LineJoin.ROUND

	if grid:
		c.set_source_rgb (0.0, 0.0, 0.0)
		c.set_line_width (1.0)
		c.set_dash ([0, 4])

		for i in range (0, SW + 1, S):
			c.move_to (i,  0)
			c.line_to (i, SH)
			c.stroke ()

		for j in range (0, SH + 1, S):
			c.move_to (0,  j)
			c.line_to (SW, j)
			c.stroke ()

		c.set_dash ([])

	# scale to user coordinates
	c.scale (S / step, S / step)
	c.set_line_width (0.0625 * step)
	font_size (c, 0.5 * step)

	return surface, c

def mark (c, name, x, y, right = True):
	print ("{} = ({:.2f}, {:.2f})".format (name, x, y))

	c.save ()
	c.translate (x, y)
	c.set_source_rgb (0.5, 0.0, 0.0)

	dot = c.get_line_width ()
	n   = 2 * dot

	c.move_to (-n, -n)
	c.line_to (+n, +n)
	c.move_to (-n, +n)
	c.line_to (+n, -n)

	c.move_to (+2 * n, +2 * n)
	c.show_text (name)

	c.stroke ()
	c.restore ()
