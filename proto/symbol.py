#!/usr/bin/python3

import cairo, math

thin   = 0.05
normal = 0.07
thick  = 0.20

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

def mark_to (c, x1, y1, kind):
	x0, y0 = c.get_current_point ()
	c.line_to (x1, y1)
	c.stroke ()

	if kind == "arrow":
		nx, ny = x0 - x1, y0 - y1
		n = 0.3 / math.hypot (nx, ny)
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

def gate (c, x, y, kind = "&", inv = True):
	c.save ()
	c.translate (x, y)

	c.move_to (0.5, 0)
	c.line_to (2.0, 0)
	c.line_to (2.0, 2)
	c.line_to (0.5, 2)
	c.line_to (0.5, 0)
	c.stroke ()

	c.move_to (0.75, 1.3)
	c.show_text (kind)

	c.move_to (2.5, 0.5)

	if inv:
		mark_to (c, 2.0, 0.5, "odot")
	else:
		c.line_to (2.0, 0.5)

	c.move_to (0.0, 0.5)
	c.line_to (0.5, 0.5)
	c.move_to (0.0, 1.5)
	c.line_to (0.5, 1.5)
	c.stroke ()

	c.restore ()

def bjt (c, x, y, npn = True):
	c.save ()
	c.translate (x, y)

	c.move_to (0.5, 1)
	arc_to (c, 2.0, 1, math.radians (180))
	arc_to (c, 0.5, 1, math.radians (180))

	c.move_to (1.0, 0.5)
	c.line_to (1.0, 1.5)
	c.move_to (0.0, 1)
	c.line_to (1.0, 1)
	c.move_to (2.0, 2)
	c.line_to (1.0, 1.25)

	if npn:
		c.move_to (1.0, 0.75)
		k = 0.5
		rx, ry = 1 * k, -0.75 * k
		mark_to (c, 1.0 + rx, 0.75 + ry, "arrow")
		c.line_to (2.0, 0)
	else:
		c.move_to (2.0, 0)
		mark_to (c, 1.0 + normal, 0.75 - normal/2, "arrow")
		c.line_to (1.0, 0.75)

	c.stroke ()
	c.restore ()

def npn (c, x, y):
	bjt (c, x, y, True)

def pnp (c, x, y):
	bjt (c, x, y, False)

