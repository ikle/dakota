#!/usr/bin/python3

import os, re, sys

from contextlib import redirect_stdout
from itertools  import count

def make_version ():
	print ('v 20250306 2')

def make_west_attr (x, y, seq, number, label, kind = 'in'):
	print ('{')
	print ('T', x + 200, y - 50, '5 8 0 1 0 8 1')
	print ('pinseq={}'.format (seq))
	print ('T', x + 200, y + 50, '5 8 1 1 0 6 1')
	print ('pinnumber={}'.format (number))
	print ('T', x + 350, y, '9 8 1 1 0 0 1')
	print ('pinlabel={}'.format (label))
	print ('T', x + 350, y, '5 8 0 1 0 2 1')
	print ('pintype={}'.format (kind))
	print ('}')

def make_east_attr (x, y, seq, number, label, kind = 'out'):
	print ('{')
	print ('T', x - 200, y - 50, '5 8 0 1 0 2 1')
	print ('pinseq={}'.format (seq))
	print ('T', x - 200, y + 50, '5 8 1 1 0 0 1')
	print ('pinnumber={}'.format (number))
	print ('T', x - 350, y, '9 8 1 1 0 6 1')
	print ('pinlabel={}'.format (label))
	print ('T', x - 350, y, '5 8 0 1 0 8 1')
	print ('pintype={}'.format (kind))
	print ('}')

def make_west_pin (x, y, seq, number, label, kind = 'in'):
	if label[0] != '!':
		print ('P', x, y, x + 300, y, '1 0 0')
	else:
		print ('V', x + 250, y, '50 6 0 0 0 -1 -1 0 -1 -1 -1 -1 -1')
		print ('P', x, y, x + 200, y, '1 0 0')

	label = re.sub (r'!(\w+)', r'\\_\1\\_', label)
	make_west_attr (x, y, seq, number, label, kind)

def make_east_pin (x, y, seq, number, label, kind = 'out'):
	if label[0] != '!':
		print ('P', x, y, x - 300, y, '1 0 0')
	else:
		print ('V', x - 250, y, '50 6 0 0 0 -1 -1 0 -1 -1 -1 -1 -1')
		print ('P', x, y, x - 200, y, '1 0 0')

	label = re.sub (r'!(\w+)', r'\\_\1\\_', label)
	make_east_attr (x, y, seq, number, label, kind)

def make_west (w, y, sn, pn, ls, kind = 'in'):
	for label in ls:
		if label == '-':
			y = y - 100
		else:
			y = y - 300
			make_west_pin (0, y, next (sn), next (pn), label, kind)
	return y

def make_east (w, y, sn, pn, ls, kind = 'in'):
	x = w + 600

	for label in ls:
		if label == '-':
			y = y - 100
		else:
			y = y - 300
			make_east_pin (x, y, next (sn), next (pn), label, kind)
	return y

def calc_len (desc):
	h = 0

	for kind, pins in desc:
		for label in pins:
			h = h + 100 if label == '-' else h + 300

	return h + 200

def make_case (w, h, label, device):
	print ('B', 300, 0, w, h, '3 0 0 0 -1 -1 0 -1 -1 -1 -1 -1')
	print ('T', 300, h + 50, '9 10 1 0 0 0 1')
	print (label)
	print ('T', 300 + w // 2, h // 2, '8 10 1 1 90 4 1')
	print ('device={}'.format (device))
	print ('T', w + 300, h + 100, '8 10 1 1 0 6 1')
	print ('refdes=U?')

def make_attr (x, y, name, value):
	print ('T', x, y, '5 8 0 0 0 0 1')
	print ('{}={}'.format (name, value))

	return y + 200

def make_attrs (w, h, c):
	y = h + 300

	if 'desc' in c:
		y = make_attr (0, y, 'description', c['desc'])

	if 'pack' in c:
		y = make_attr (0, y, 'footprint', c['pack'])

	if 'author' in c:
		y = make_attr (0, y, 'author', c['author'])

	n = len (c['slots'])

	if n < 2:
		y = make_attr (0, y, 'numslots', 0)
	else:
		y = make_attr (0, y, 'numslots', n)
		y = make_attr (0, y, 'slot', 1)
		i = 1

		for slot in c['slots']:
			slotdef = str (i) + ':' + ','.join (map (str, slot))
			y = make_attr (0, y, 'slotdef', slotdef)
			i = i + 1

	if 'nets' in c:
		for net in c['nets']:
			y = make_attr (0, y, 'net', net)

def make_device (c, path):
	(w, h) = (1200, max (calc_len (c['east']), calc_len (c['west'])))
	(sn, pn) = (count (1, 1), iter (c['slots'][0]))

	root = os.path.dirname (path)

	if root != '':
		os.makedirs (root, exist_ok = True)

	with open (path, 'w') as to:
		with redirect_stdout (to):
			make_version ()

			if 'east' in c:
				y = h

				for kind, pins in c['east']:
					y = make_west (w, y, sn, pn, pins, kind)

			if 'west' in c:
				y = h

				for kind, pins in c['west']:
					y = make_east (w, y, sn, pn, pins, kind)

			make_case  (w, h, c['hint'], c['dev'])
			make_attrs (w, h, c)

LCX244 = {
	'author': 'Alexei A Smekalkine <ikle@ikle.ru>',

	'hint':	'244',
	'dev':	'74LCX244',
	'desc':	'Octal 3-state Buffer',
	'pack':	'DIP20',

	'east': [('in',  ['D0', 'D1', 'D2', 'D3', '-', '!OE'])],
	'west': [('tri', ['Y0', 'Y1', 'Y2', 'Y3'])],
	'nets': ['Vcc:20', 'GND:10'],

	'slots': [[ 2,  4,  6,  8,  1, 18, 16, 14, 12],
		  [11, 13, 15, 17, 19,  9,  7,  5,  3]]
}

LCX245 = {
	'author': 'Alexei A Smekalkine <ikle@ikle.ru>',

	'hint':	'245',
	'dev':	'74LCX245',
	'desc':	'3-state Octal Bus Transceiver',
	'pack':	'DIP20',

	'east': [('io',  ['A0', 'A1', 'A2', 'A3', 'A4', 'A5', 'A6', 'A7', '-']), ('in', ['!OE', 'T/!R'])],
	'west': [('io',  ['B0', 'B1', 'B2', 'B3', 'B4', 'B5', 'B6', 'B7'])],
	'nets': ['Vcc:20', 'GND:10'],

	'slots': [[2, 3, 4, 5, 6, 7, 8, 9, 19, 1, 18, 17, 16, 15, 14, 13, 12, 11]]
}

LCX373 = {
	'author': 'Alexei A Smekalkine <ikle@ikle.ru>',

	'hint':	'373',
	'dev':	'74LCX373',
	'desc':	'Octal Transparent D-Type Latch with 3-state Outputs',
	'pack':	'DIP20',

	'east': [('in',  ['D0', 'D1', 'D2', 'D3', 'D4', 'D5', 'D6', 'D7', '-', '!OE', 'LE'])],
	'west': [('tri', ['Q0', 'Q1', 'Q2', 'Q3', 'Q4', 'Q5', 'Q6', 'Q7'])],
	'nets': ['Vcc:20', 'GND:10'],

	'slots': [[3, 4, 7, 8, 13, 14, 17, 18, 1, 11, 2, 5, 6, 9, 12, 15, 16, 19]]
}

LCX541 = {
	'author': 'Alexei A Smekalkine <ikle@ikle.ru>',

	'hint':	'541',
	'dev':	'74LCX541',
	'desc':	'Octal Buffer/Driver with 3-state Outputs',
	'pack':	'DIP20',

	'east': [('in',  ['D0', 'D1', 'D2', 'D3', 'D4', 'D5', 'D6', 'D7', '-', '!OE1', '!OE2'])],
	'west': [('tri', ['Y0', 'Y1', 'Y2', 'Y3', 'Y4', 'Y5', 'Y6', 'Y7'])],
	'nets': ['Vcc:20', 'GND:10'],

	'slots': [[2, 3, 4, 5, 6, 7, 8, 9, 1, 19, 18, 17, 16, 15, 14, 13, 12, 11]]
}

LCX573 = {
	'author': 'Alexei A Smekalkine <ikle@ikle.ru>',

	'hint':	'573',
	'dev':	'74LCX573',
	'desc':	'Octal Transparent D-Type Latch with 3-state Outputs',
	'pack':	'DIP20',

	'east': [('in',  ['D0', 'D1', 'D2', 'D3', 'D4', 'D5', 'D6', 'D7', '-', '!OE', 'LE'])],
	'west': [('tri', ['Q0', 'Q1', 'Q2', 'Q3', 'Q4', 'Q5', 'Q6', 'Q7'])],
	'nets': ['Vcc:20', 'GND:10'],

	'slots': [[2, 3, 4, 5, 6, 7, 8, 9, 1, 11, 19, 18, 17, 16, 15, 14, 13, 12]]
}

make_device (LCX244, 'lib/ttl-244-1.sym')
make_device (LCX245, 'lib/ttl-245-1.sym')
make_device (LCX373, 'lib/ttl-373-1.sym')
make_device (LCX541, 'lib/ttl-541-1.sym')
make_device (LCX573, 'lib/ttl-573-1.sym')

SRAM_64K_8 = {
	'author': 'Alexei A Smekalkine <ikle@ikle.ru>',

	'hint':	'64Kx8',
	'dev':	'UM61512A',
	'desc':	'64K-word x 8-bit high-speed CMOS static RAM',
	'pack':	'DIP32',

	'east':	[('in',  ['A0', 'A1',  'A2',  'A3',  'A4',  'A5',  'A6',  'A7']),
		 ('in',  ['A8', 'A9', 'A10', 'A11', 'A12', 'A13', 'A14', 'A15']),
		 ('in',  ['-', '!WE', '!OE', '!CE1', 'CE2'])],
	'west':	[('io',  ['D0', 'D1', 'D2', 'D3', 'D4', 'D5', 'D6', 'D7'])],
	'nets': ['Vcc:32', 'GND:16'],

	'slots': [[12, 11, 10, 9, 8, 7, 6, 5, 27, 26, 23, 25, 4, 28, 3, 31,
		   29, 24, 22, 30,
		   13, 14, 15, 17, 18, 19, 20, 21]]
}

make_device (SRAM_64K_8,  'lib/sram-64K-8-1.sym')

