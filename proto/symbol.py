#!/usr/bin/python3

import cairo, math, sheet

def arc_split (c, x1, y1, angle): 
	x0, y0 = c.get_current_point ()

	# calculate middle point

	ta  = math.tan (angle * 0.5)
	cta = 1 / ta
	sa  = math.sin (angle * 0.5)
	csa = 1 / sa

	vox, voy = (x1 - x0) * 0.5, (y1 - y0) * 0.5
	ox,  oy  = x0 + vox, y0 + voy
	vnx, vny = -voy, vox
	vrx, vry = vnx * cta, vny * cta
	rx,  ry  = ox + vrx, oy + vry
	mrx, mry = vnx * csa, vny * csa  # from M to R
	mx,  my  = rx - mrx, ry - mry

	return mx, my, mrx, mry

def quad_to (c, x1, y1, x2, y2):
	x0, y0 = c.get_current_point ()
	k = 2.0 / 3.0

	ax, ay = x0 + k * (x1 - x0), y0 + k * (y1 - y0)
	bx, by = x2 + k * (x1 - x2), y2 + k * (y1 - y2)

	c.curve_to (ax, ay, bx, by, x2, y2)

def arc_draw (c, mrx, mry, mx, my, x1, y1, angle):
	tb  = math.tan (angle * 0.25)
	ctb = 1 / tb

	vqx, vqy = -mry * tb, mrx * tb  # MR, π/2 rotated and scaled
	ax,  ay  = mx + vqx, my + vqy
	bx,  by  = mx - vqx, my - vqy

	quad_to (c, ax, ay, mx, my)
	quad_to (c, bx, by, x1, y1)

def arc_to (c, x1, y1, angle):
	mx, my, mrx, mry = arc_split (c, x1, y1, angle)

	if angle <= 1.05:  # ~60°
		arc_draw (c, mrx, mry, mx, my, x1, y1, angle)
	else:
		angle /= 2
		arc_to (c, mx, my, angle)
		arc_to (c, x1, y1, angle)

def mark_to (c, x1, y1, kind, scale = 1):
	x0, y0 = c.get_current_point ()
	c.line_to (x1, y1)
	c.stroke ()

	if kind == "arrow":
		nx, ny = x0 - x1, y0 - y1
		n = 0.3 * scale / math.hypot (nx, ny)
		nx, ny = n * nx, n * ny
		ox, oy = x1 + nx, y1 + ny

		nx, ny = 0.5 * nx, 0.5 * ny
		ax, ay = ox - ny, oy + nx
		bx, by = ox + ny, oy - nx

		c.move_to (x1, y1)
		c.line_to (ax, ay)
		c.line_to (bx, by)
		c.close_path ()
		c.fill_preserve ()
		c.stroke ()

		c.move_to (x1, y1)

	elif kind == "dot":
		c.save ()
		c.move_to (x1, y1)
		c.line_to (x1, y1)
		c.scale (4, 4)
		c.stroke ()
		c.restore ()

		c.move_to (x1, y1)

	elif kind == "odot":
		c.move_to (x1, y1)
		c.line_to (x1, y1)
		c.save ()
		c.scale (5, 5)
		c.stroke_preserve ()
		c.restore ()
		c.save ()
		c.set_source_rgb (1.0, 1.0, 1.0)
		c.scale (3, 3)
		c.stroke ()
		c.restore ()

		c.move_to (x1, y1)

def enter (c, x, y):
	c.save ()
	c.translate (x, y)
	width = c.get_line_width ()
	c.scale (0.125, 0.125)
	c.set_line_width (width * 8)
	c.set_font_face (cairo.ToyFontFace ("monospace"))
	sheet.font_size (c, 4.5)

def leave (c):
	c.stroke ()
	c.restore ()

def gate (c, x, y, kind = "&", inv = True):
	enter (c, x, y)

	c.move_to (4,   0)
	c.line_to (16,  0)
	c.line_to (16, 16)
	c.line_to (4,  16)
	c.line_to (4,   0)
	c.stroke ()

	c.move_to (6, 10)
	c.show_text (kind)

	c.move_to (20, 4)

	if inv:
		mark_to (c, 16, 4, "odot")
	else:
		c.line_to (16, 4)

	c.move_to (0, 4)
	c.line_to (4, 4)
	c.move_to (0, 12)
	c.line_to (4, 12)

	leave (c)

def resistor (c, x, y):
	enter (c, x, y)

	c.move_to (0, 4)
	c.line_to (4, 4)

	c.move_to (16, 4)
	c.line_to (12, 4)

	c.move_to (4,  2)
	c.line_to (4,  6)
	c.line_to (12, 6)
	c.line_to (12, 2)
	c.close_path ()

	leave (c)

def capasitor (c, x, y):
	enter (c, x, y)

	c.move_to (0, 4)
	c.line_to (7, 4)

	c.move_to (16, 4)
	c.line_to (9, 4)

	c.move_to (7, 0)
	c.line_to (7, 8)

	c.move_to (9, 0)
	c.line_to (9, 8)

	leave (c)

def diode (c, x, y):
	enter (c, x, y)

	c.move_to (0,  4)
	c.line_to (16, 4)

	c.move_to (6,  6)
	c.line_to (10, 4)
	c.line_to (6,  2)
	c.close_path ()

	c.move_to (10, 6)
	c.line_to (10, 2)

	leave (c)

def bjt (c, x, y, npn = True):
	enter (c, x, y)

	c.move_to (4,  8)
	arc_to (c, 16, 8, math.radians (180))
	arc_to (c, 4,  8, math.radians (180))

	c.move_to (8,  4)
	c.line_to (8,  12)
	c.move_to (0,  8)
	c.line_to (8,  8)
	c.move_to (16, 16)
	c.line_to (8,  10)

	if npn:
		c.move_to (8, 6)
		mark_to (c, 12, 3, "arrow", 8)
		c.line_to (16, 0)
	else:
		c.move_to (16, 0)
		dot = c.get_line_width ()
		mark_to (c, 8 + dot, 6 - dot/2, "arrow", 8)
		c.line_to (8, 6)

	leave (c)

def npn (c, x, y):
	bjt (c, x, y, True)

def pnp (c, x, y):
	bjt (c, x, y, False)

