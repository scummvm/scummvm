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
/*#include "Crt.h"*/

/*$R+*/

namespace Avalanche {

typedef matrix < '\0', '\377', 0, 15, byte > fonttype;

integer gd, gm;
text t;
array<1, 2, fonttype> f;
file<fonttype> ff;
array<1, 80, byte> l;
varying_string<80> x;

void do_one(byte z) {
	byte a /*absolute $A000:0*/;
	byte aa /*absolute $A000:80*/;
	byte az /*absolute $A000:27921*/;
	;
	a = getpixel(0, 0);
	move(l, az, z);

	port[0x3c5] = 8;
	port[0x3cf] = 0;
	move(aa, a, 27920);
}

void background() {
	byte y;
	;
	for (y = 1; y <= 15; y ++) {
		;
		setcolor(y);
		outtextxy(17, y * 12, "Jesus is Lord!");
	}
}

void scroll(string z) {
	byte x, y, lz;
	char c;
	;
	fillchar(l, 80, '\0');
	if (z == "")
		for (y = 1; y <= 12; y ++) do_one(0);
	c = z[1];
	Delete(z, 1, 1);
	lz = length(z);
	switch (c) {
	case '*':
		for (y = 0; y <= 15; y ++) {
			;
			for (x = 1; x <= lz; x ++)
				l[x] = f[2][z[x]][y];
			do_one(lz);
		}
		break;
	case '>': {
		;
		lz += 7;
		for (y = 0; y <= 13; y ++) {
			;
			for (x = 1; x <= lz; x ++)
				l[x + 7] = f[1][z[x]][y];
			do_one(lz);
			do_one(lz); /* double-height characters */
		}
	}
	break;
	}
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	;
	gd = 3;
	gm = 1;
	initgraph(gd, gm, "");
	port[0x3c4] = 2;
	port[0x3cf] = 4;
	assign(ff, "avalot.fnt");
	reset(ff);
	ff >> f[1];
	close(ff);
	assign(ff, "avitalic.fnt");
	reset(ff);
	ff >> f[2];
	close(ff);
	assign(t, "credits.txt");
	reset(t);
	background();
	for (gd = 8; gd <= 15; gd ++) setpalette(gd, 62);
	do {
		t >> x >> NL;
		scroll(x);
	} while (!(eof(t) || keypressed()));
	close(t);
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.