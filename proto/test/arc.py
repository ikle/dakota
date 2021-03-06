#!/usr/bin/python3

import cairo, sheet, math
import symbol as s

from sheet import mark

def arc_split (c, x1, y1, angle, debug = False):
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

	if debug:
		mark (c, "P0",  x0,  y0)
		mark (c, "P1",  x1,  y1)
		mark (c, "O ",  ox,  oy)
		mark (c, "R ",  rx,  ry)
		mark (c, "M ",  mx,  my)
		c.move_to (x0, y0)

	return mx, my, mrx, mry

def arc_draw (c, mrx, mry, mx, my, x1, y1, angle, debug = False):
	tb  = math.tan (angle * 0.25)
	ctb = 1 / tb

	vqx, vqy = -mry * tb, mrx * tb  # MR, π/2 rotated and scaled
	ax,  ay  = mx + vqx, my + vqy
	bx,  by  = mx - vqx, my - vqy

	if not debug:
		s.quad_to (c, ax, ay, mx, my)
		s.quad_to (c, bx, by, x1, y1)
	if debug:
		mark (c, "A ",  ax,  ay)
		mark (c, "B ",  bx,  by)

def arc_to (c, x1, y1, angle, debug = False):
	mx, my, mrx, mry = arc_split (c, x1, y1, angle, debug)

	if angle <= 1.05:
		arc_draw (c, mrx, mry, mx, my, x1, y1, angle, debug)
	else:
		angle /= 2
		arc_to (c, mx, my, angle, debug)
		arc_to (c, x1, y1, angle, debug)

	if debug:
		c.move_to (x1, y1)
