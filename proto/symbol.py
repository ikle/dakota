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

	elif kind == "dot":
		c.save ()
		c.move_to (x1, y1)
		c.line_to (x1, y1)
		c.scale (4, 4)
		c.stroke ()
		c.restore ()

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

	elif kind == "rise":
		nx, ny = x1 - x0, y1 - y0
		n = 0.125 * scale / math.hypot (nx, ny)
		nx, ny = n * nx, n * ny

		k = 1  # √2/2 * √2 = 1
		vx, vy = k * nx - k * ny, k * nx + k * ny  # 45° left

		c.move_to (x1 - vx, y1 - vy)
		c.line_to (x1 + vx, y1 + vy)

	elif kind == "fall":
		nx, ny = x1 - x0, y1 - y0
		n = 0.125 * scale / math.hypot (nx, ny)
		nx, ny = n * nx, n * ny

		k = 1  # √2/2 * √2 = 1
		vx, vy = k * nx + k * ny, -k * nx + k * ny  # 45° right

		c.move_to (x1 - vx, y1 - vy)
		c.line_to (x1 + vx, y1 + vy)

	c.move_to (x1, y1)

# context guards

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

# core API

def start (c, x, y):
	c.move_to (x, y)

def move (c, dx, dy):
	c.rel_move_to (dx, dy)

def line (c, dx, dy):
	c.rel_line_to (dx, dy)

def arc (c, dx, dy, angle):
	x, y = c.get_current_point ()
	arc_to (c, x + dx, y + dy, math.radians (angle))

def mark (c, dx, dy, kind):
	x, y = c.get_current_point ()
	mark_to (c, x + dx, y + dy, kind)

def close (c):
	c.close_path ()

def text (c, s):
	c.show_text (s)

# test tiles and symbols

def gate (c, x, y, kind = "&", inv = True):
	enter (c, x, y)

	start (c,   4,   0)
	line  (c,  12,   0)
	line  (c,   0,  16)
	line  (c, -12,   0)
	close (c)

	start (c, 6, 10)
	text  (c, kind)

	start (c, 20, 4)

	if inv:
		mark (c, -4, 0, "odot")
	else:
		line (c, -4, 0)

	start (c, 0, 4)
	line  (c, 4, 0)
	start (c, 0, 12)
	line  (c, 4, 0)

	leave (c)

def resistor (c, x, y):
	enter (c, x, y)

	start (c, 0, 4)
	line  (c, 4, 0)

	start (c, 16, 4)
	line  (c, -4, 0)

	start (c, 4,  2)
	line  (c, 0,  4)
	line  (c, 8,  0)
	line  (c, 0, -4)
	close (c)

	leave (c)

def capasitor (c, x, y, polar = False):
	enter (c, x, y)

	start (c, 0, 4)
	line  (c, 7, 0)

	start (c, 16, 4)
	line  (c, -7, 0)

	start (c, 7, 0)
	line  (c, 0, 8)

	start (c, 9, 0)
	line  (c, 0, 8)

	if polar:
		start (c, 11, 6)
		line  (c,  0, 2)
		start (c, 10, 7)
		line  (c,  2, 0)

	leave (c)

def inductor (c, x, y, core = False):
	enter (c, x, y)

	start (c, 0, 4)
	line  (c, 4, 0)

	arc (c, 4, 0, -180)
	arc (c, 4, 0, -180)
	arc (c, 4, 0, -180)
#	arc (c, 4, 0, -180)

	line (c, 4, 0)

	if core:
		start (c, 4, 7)
		line  (c, 12, 0)

	leave (c)

def diode (c, x, y):
	enter (c, x, y)

	start (c, 0,  4)
	line  (c, 16, 0)

	start (c,  6,  6)
	line  (c,  4, -2)
	line  (c, -4, -2)
	close (c)

	start (c, 10,  6)
	line  (c,  0, -4)

	leave (c)

def bjt_case (c):
	c.move_to (4,  8)
	arc_to (c, 16, 8, math.radians (180))
	arc_to (c, 4,  8, math.radians (180))

def bjt_base (c):
	c.move_to (8,  4)
	c.line_to (8,  12)
	c.move_to (0,  8)
	c.line_to (8,  8)

def bjt_cu (c):
	c.move_to (8,  10)
	c.line_to (12, 13)
	c.line_to (12, 18)

def bjt_cd (c):
	c.move_to (8,  6)
	c.line_to (12, 3)
	c.line_to (12, -2)

def bjt_enu (c):
	c.move_to (8, 10)
	mark_to (c, 12, 13, "arrow", 8)
	c.line_to (12, 18)

def bjt_end (c):
	c.move_to (8, 6)
	mark_to (c, 12, 3, "arrow", 8)
	c.line_to (12, -2)

def bjt_epu (c):
	c.move_to (12, 18)
	c.line_to (12, 13)
	dot = c.get_line_width ()
	mark_to (c, 8 + dot, 10 + (3/4) * dot, "arrow", 8)
#	c.line_to (8, 10)

def bjt_epd (c):
	c.move_to (12, -2)
	c.line_to (12, 3)
	dot = c.get_line_width ()
	mark_to (c, 8 + dot, 6 - (3/4) * dot, "arrow", 8)
#	c.line_to (8, 6)

def npn_ce (c, x, y):
	enter (c, x, y)

	bjt_case (c)
	bjt_base (c)
	bjt_cu   (c)
	bjt_end  (c)

	leave (c)

def npn_ec (c, x, y):
	enter (c, x, y)

	bjt_case (c)
	bjt_base (c)
	bjt_cd   (c)
	bjt_enu  (c)

	leave (c)

def pnp_ce (c, x, y):
	enter (c, x, y)

	bjt_case (c)
	bjt_base (c)
	bjt_cu   (c)
	bjt_epd  (c)

	leave (c)

def pnp_ec (c, x, y):
	enter (c, x, y)

	bjt_case (c)
	bjt_base (c)
	bjt_cd   (c)
	bjt_epu  (c)

	leave (c)

