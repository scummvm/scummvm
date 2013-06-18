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

namespace Avalanche {

integer gd, gm;

void zoomout(integer x, integer y) {
	integer x1, y1, x2, y2;
	byte fv;
	;
	setcolor(white);
	setwritemode(xorput);
	setlinestyle(dottedln, 0, 1);

	for (fv = 1; fv <= 20; fv ++) {
		;
		x1 = x - (x / 20) * fv;
		y1 = y - ((y - 10) / 20) * fv;
		x2 = x + (((639 - x) / 20) * fv);
		y2 = y + (((161 - y) / 20) * fv);

		rectangle(x1, y1, x2, y2);
		delay(17);
		rectangle(x1, y1, x2, y2);
	}
}

void zoomin(integer x, integer y) {
	integer x1, y1, x2, y2;
	byte fv;
	;
	setcolor(white);
	setwritemode(xorput);
	setlinestyle(dottedln, 0, 1);

	for (fv = 20; fv >= 1; fv --) {
		;
		x1 = x - (x / 20) * fv;
		y1 = y - ((y - 10) / 20) * fv;
		x2 = x + (((639 - x) / 20) * fv);
		y2 = y + (((161 - y) / 20) * fv);

		rectangle(x1, y1, x2, y2);
		delay(17);
		rectangle(x1, y1, x2, y2);
	}
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	;
	gd = 3;
	gm = 0;
	initgraph(gd, gm, "c:\\bp\\bgi");
	zoomout(177, 77);
	zoomout(7, 31);
	zoomout(577, 124);
	zoomout(320, 85);
	zoomin(177, 77);
	zoomin(7, 31);
	zoomin(577, 124);
	zoomin(320, 85);
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.