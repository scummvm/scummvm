/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#include "graph.h"
/*#include "Binu.h"*/
/*#include "Crt.h"*/

namespace Avalanche {

/* ...gogogoch */struct cursor {
	matrix<0, 1, 0, 15, word> mask;
	integer horzhotspot, verthotspot;
};
typedef   /* ha ha */matrix<0, 1, 0, 15, varying_string<16> > precursor;

const array<0, 3, byte> colours = {{darkgray, blue, white, green}};

cursor c_current;
boolean usepointer;
precursor cpc;
byte hhs, vhs;
string fn, desc;
integer gd, gm;

void recalc() {
	byte a, b;
	;
	{
		;
		for (a = 0; a <= 1; a ++)
			for (b = 0; b <= 15; b ++)
				c_current.mask[a][b] = bintoword(cpc[a][b]);
		c_current.horzhotspot = hhs;
		c_current.verthotspot = vhs;
	}
}

void load() {
	text t;
	string x;
	byte a, b;
	;
	assign(t, fn);
	reset(t);
	do {
		t >> x >> NL;
	} while (!(x == '|')); /* bypass junk */
	t >> desc >> NL;
	for (a = 0; a <= 1; a ++)
		for (b = 0; b <= 15; b ++)
			t >> cpc[a][b] >> NL;
	t >> hhs >> NL;
	t >> vhs >> NL;
	close(t);
	recalc();
}

void display() {
	const integer o0 = ord('0');
	const integer o1 = ord('1');
	byte x, y, p1, p2;
	;
	for (y = 0; y <= 15; y ++) {
		;
		for (x = 1; x <= 16; x ++) {
			;
			switch (ord(cpc[0][y][x]) * 2 + ord(cpc[1][y][x])) {
			case o0*2+o0: {
				;
				p1 = 0;
				p2 = 0;
			}
			break; /* p1= silhouette, p2= real */
			case o0*2+o1: {
				;
				p1 = 0;
				p2 = 15;
			}
			break;
			case o1*2+o0: {
				;
				p1 = 15;
				p2 = 0;
			}
			break;
			case o1*2+o1: {
				;
				p1 = 9;
				p2 = 9;
			}
			break; /* invalid- can't use this */
			}
			putpixel(x - 1, y, p1);
			putpixel(x + 99, y, p2);
		}
		output << NL;
	}
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	;
	gd = 3;
	gm = 1;
	initgraph(gd, gm, "");
	fn = "d:screwdri.inc";
	load();
	display();
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.