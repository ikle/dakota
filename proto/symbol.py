#!/usr/bin/python3

import cairo, math

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
		c.scale (3, 3)
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

