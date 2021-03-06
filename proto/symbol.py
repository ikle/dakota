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

def show_arrow (c, x, y, dx, dy):
	n = -2.4 / math.hypot (dx, dy)
	dx, dy = n * dx, n * dy
	ox, oy = x + dx, y + dy

	dx, dy = 0.5 * dx, 0.5 * dy
	ax, ay = ox - dy, oy + dx
	bx, by = ox + dy, oy - dx

	c.move_to (x, y)
	c.line_to (ax, ay)
	c.line_to (bx, by)
	c.close_path ()
	c.fill_preserve ()
	c.stroke ()

def show_pointer (c, x, y, dx, dy):
	n = -0.5 / math.hypot (dx, dy)
	x, y = x + n * dx, y + n * dy

	show_arrow (c, x, y, dx, dy)

def show_dot (c, x, y):
	c.move_to (x, y)
	c.line_to (x, y)

	c.save ()
	c.scale (4, 4)
	c.stroke ()
	c.restore ()

def show_odot (c, x, y):
	c.move_to (x, y)
	c.line_to (x, y)

	c.save ()
	c.scale (5, 5)
	c.stroke_preserve ()
	c.restore ()
	c.save ()
	c.set_source_rgb (1.0, 1.0, 1.0)
	c.scale (3, 3)
	c.stroke ()
	c.restore ()

def show_rise (c, x, y, dx, dy):
	n = 1.0 / math.hypot (dx, dy)
	dx, dy = n * dx, n * dy

	k = 1  # √2/2 * √2 = 1
	vx, vy = k * dx - k * dy, k * dx + k * dy  # 45° left

	c.move_to (x - vx, y - vy)
	c.line_to (x + vx, y + vy)

def show_fall (c, x, y, dx, dy):
	n = 1.0 / math.hypot (dx, dy)
	dx, dy = n * dx, n * dy

	k = 1  # √2/2 * √2 = 1
	vx, vy = k * dx + k * dy, -k * dx + k * dy  # 45° right

	c.move_to (x - vx, y - vy)
	c.line_to (x + vx, y + vy)

def mark_to (c, x1, y1, kind):
	x0, y0 = c.get_current_point ()
	c.line_to (x1, y1)
	c.stroke ()

	if kind == "arrow":
		show_arrow (c, x1, y1, x1 - x0, y1 - y0)

	if kind == "pointer":
		show_pointer (c, x1, y1, x1 - x0, y1 - y0)

	elif kind == "dot":
		show_dot (c, x1, y1)

	elif kind == "odot":
		show_odot (c, x1, y1)

	elif kind == "rise":
		show_rise (c, x1, y1, x1 - x0, y1 - y0)

	elif kind == "fall":
		show_fall (c, x1, y1, x1 - x0, y1 - y0)

	c.move_to (x1, y1)

# context guards

def enter (c, x, y):
	c.save ()
	c.translate (x, y)
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

def cross (c, x, y):
	enter (c, x, y)

	start (c, 0, 2)
	mark  (c, 2, 0, "dot")
	line  (c, 2, 0)

	start (c, 2, 0)
	line  (c, 0, 4)

	leave (c)

def gate (c, x, y, kind = "&", inv = True):
	enter (c, x, y)

	start (c,   2,   0)
	line  (c,  12,   0)
	line  (c,   0,  16)
	line  (c, -12,   0)
	close (c)

	start (c, 4, 10)
	text  (c, kind)

	start (c, 16, 4)

	if inv:
		mark (c, -2, 0, "odot")
	else:
		line (c, -2, 0)

	start (c, 0, 4)
	line  (c, 2, 0)
	start (c, 0, 12)
	line  (c, 2, 0)

	leave (c)

def resistor (c, x, y):
	enter (c, x, y)

	start (c, 0, 4)
	line  (c, 2, 0)

	start (c, 12, 4)
	line  (c, -2, 0)

	start (c, 2,  2)
	line  (c, 0,  4)
	line  (c, 8,  0)
	line  (c, 0, -4)
	close (c)

	leave (c)

def capasitor (c, x, y, polar = False):
	enter (c, x, y)

	start (c, 0, 6)
	line  (c, 5, 0)

	start (c, 12, 6)
	line  (c, -5, 0)

	start (c, 5, 2)
	line  (c, 0, 8)

	start (c, 7, 2)
	line  (c, 0, 8)

	if polar:
		start (c, 9, 8)
		line  (c, 0, 2)
		start (c, 8, 9)
		line  (c, 2, 0)

	leave (c)

def inductor (c, x, y, core = False):
	enter (c, x, y)

	start (c, 0, 4)
	line  (c, 2, 0)

	arc (c, 4, 0, -180)
	arc (c, 4, 0, -180)
	arc (c, 4, 0, -180)
#	arc (c, 4, 0, -180)

	line (c, 2, 0)

	if core:
		start (c,  2, 7)
		line  (c, 12, 0)

	leave (c)

def diode (c, x, y):
	enter (c, x, y)

	start (c, 0,  4)
	line  (c, 12, 0)

	start (c,  4,  6)
	line  (c,  4, -2)
	line  (c, -4, -2)
	close (c)

	start (c, 8,  6)
	line  (c, 0, -4)

	leave (c)

def bjt_case (c):
	start (c,   2, 8)
	arc   (c,  12, 0, 180)
	arc   (c, -12, 0, 180)

def bjt_base (c):
	start (c, 6,  4)
	line  (c, 0,  8)
	start (c, 0,  8)
	line  (c, 6,  0)

def bjt_cu (c):
	start (c, 6, 10)
	line  (c, 4,  3)
	line  (c, 0,  3)

def bjt_cd (c):
	start (c, 6,  6)
	line  (c, 4, -3)
	line  (c, 0, -3)

def bjt_enu (c):
	start (c, 6, 10)
	mark  (c, 4,  3, "arrow")
	line  (c, 0,  3)

def bjt_end (c):
	start (c, 6,  6)
	mark  (c, 4, -3, "arrow")
	line  (c, 0, -3)

def bjt_epu (c):
	start (c, 10, 16)
	line  (c,  0, -3)
	mark  (c, -4, -3, "pointer")
#	dot = c.get_line_width ()
#	mark  (c, -4 + dot, -3 + (3/4) * dot, "arrow")
#	line_abs (c, 8, 10)

def bjt_epd (c):
	start (c, 10,  0)
	line  (c,  0,  3)
	mark  (c, -4,  3, "pointer")
#	dot = c.get_line_width ()
#	mark  (c, -4 + dot, 3 - (3/4) * dot, "arrow")
#	line_abs (c, 8, 6)

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

def jfet_ng (c):
	start (c, 0, 5)
	mark  (c, 8, 0, "pointer")

def jfet_pg (c):
	start (c,  8, 5)
	mark  (c, -4, 0, "arrow")
	line  (c, -4, 0)

def fet_base (c):
	start (c, 8, 4)
	line  (c, 0, 8)

	start (c, 8,  5)
	line  (c, 4,  0)
	line  (c, 0, -5)

	start (c, 8, 11)
	line  (c, 4,  0)
	line  (c, 0,  5)

def nfet_ds (c, x, y):
	enter (c, x, y)

	bjt_case (c)
	fet_base (c)
	jfet_ng  (c)

	leave (c)

def pfet_ds (c, x, y):
	enter (c, x, y)

	bjt_case (c)
	fet_base (c)
	jfet_pg  (c)

	leave (c)

def mos_base (c):
	start (c, 7, 4)
	line  (c, 0, 2)
	start (c, 7, 7)
	line  (c, 0, 2)
	start (c, 7, 10)
	line  (c, 0, 2)

	start (c, 7,  5)
	line  (c, 4,  0)
	line  (c, 0, -5)

	start (c, 7, 11)
	line  (c, 4,  0)
	line  (c, 0,  5)

def mos_n (c):
	start (c, 11, 8)
	mark  (c, -4, 0, "pointer")

def mos_p (c):
	start (c,  7, 8)
	mark  (c,  4, 0, "arrow")

def mos_gd (c):			# gate down
	start (c, 0, 5)
	line  (c, 5, 0)
	line  (c, 0, 6)

def mos_gu (c):			# gate up
	start (c, 0, 11)
	line  (c, 5,  0)
	line  (c, 0, -6)

def mos_bd (c):			# bulk down
	start (c, 11,  8)
	line  (c,  0, -3)

def mos_bu (c):			# bulk up
	start (c, 11,  8)
	line  (c,  0,  3)

def mos_bf (c):			# bulk free
	start (c, 11,  8)
	line  (c,  5,  0)

def nmos_ds (c, x, y):
	enter (c, x, y)

	bjt_case (c)
	mos_base (c)
	mos_gd   (c)
	mos_bd   (c)
	mos_n    (c)

	leave (c)

def nmos_sd (c, x, y):
	enter (c, x, y)

	bjt_case (c)
	mos_base (c)
	mos_gu   (c)
	mos_bu   (c)
	mos_n    (c)

	leave (c)

def pmos_ds (c, x, y):
	enter (c, x, y)

	bjt_case (c)
	mos_base (c)
	mos_gd   (c)
	mos_bd   (c)
	mos_p    (c)

	leave (c)

def pmos_sd (c, x, y):
	enter (c, x, y)

	bjt_case (c)
	mos_base (c)
	mos_gu   (c)
	mos_bu   (c)
	mos_p    (c)

	leave (c)

def nmos_dbs (c, x, y):
	enter (c, x, y)

	bjt_case (c)
	mos_base (c)
	mos_gd   (c)
	mos_bf   (c)
	mos_n    (c)

	leave (c)

def pmos_dbs (c, x, y):
	enter (c, x, y)

	bjt_case (c)
	mos_base (c)
	mos_gd   (c)
	mos_bf   (c)
	mos_p    (c)

	leave (c)

def nmos_sbd (c, x, y):
	enter (c, x, y)

	bjt_case (c)
	mos_base (c)
	mos_gu   (c)
	mos_bf   (c)
	mos_n    (c)

	leave (c)

def pmos_sbd (c, x, y):
	enter (c, x, y)

	bjt_case (c)
	mos_base (c)
	mos_gu   (c)
	mos_bf   (c)
	mos_p    (c)

	leave (c)

