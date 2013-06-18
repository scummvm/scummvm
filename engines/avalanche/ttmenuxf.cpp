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
/*#include "Tommys.h"*/
/*#include "Crt.h"*/

namespace Avalanche {

integer gd, gm;
word s;
pointer p;
file<byte> f;
byte bit;

void load() {   /* Load2, actually */
	byte a0;  /*absolute $A000:800;*/
	byte a1;  /*absolute $A000:17184;*/
	byte bit;
	untyped_file f;
	varying_string<2> xx;
	boolean was_virtual;

	assign(f, "v:ttmenu.avd");
	reset(f, 1);
	seek(f, 177);
	for (bit = 0; bit <= 3; bit ++) {
		port[0x3c4] = 2;
		port[0x3ce] = 4;
		port[0x3c5] = 1 << bit;
		port[0x3cf] = bit;
		blockread(f, a0, 12080);
	}
	close(f);
	bit = getpixel(0, 0);
}

void finder() {
	char r;
	integer x, y;
	;
	setfillstyle(0, 0);
	setcolor(14);
	x = 320;
	y = 100;
	setwritemode(xorput);
	do {
		bar(0, 0, 200, 10);
		outtextxy(0, 0, strf(x) + ',' + strf(y));
		line(x - 20, y, x + 20, y);
		line(x, y - 20, x, y + 20);
		do {
			;
		} while (!keypressed());
		line(x - 20, y, x + 20, y);
		line(x, y - 20, x, y + 20);
		switch (readkey()) {
		case '\15':
			return;
			break;
		case '8':
			y -= 10;
			break;
		case '4':
			x -= 10;
			break;
		case '6':
			x += 10;
			break;
		case '2':
			y += 10;
			break;
		case '\0':
			switch (readkey()) {
			case cup:
				y -= 1;
				break;
			case cdown:
				y += 1;
				break;
			case cleft:
				x -= 1;
				break;
			case cright:
				x += 1;
				break;
			}
			break;
		}
	} while (!false);
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	gd = 3;
	gm = 1;
	initgraph(gd, gm, "");
	load();
	finder();
	s = imagesize(342, 21, 407, 119);
	getmem(p, s);
	getimage(342, 21, 407, 119, p);
	putimage(342, 21, p, 4);
	input >> NL;
	putimage(264, 120, p, 0);
	input >> NL;
	freemem(p, s);

	s = imagesize(264, 12, 329, 217);
	getmem(p, s);
	getimage(264, 21, 329, 218, p);
	putimage(264, 21, p, 4);
	putimage(0, 0, p, 0);
	freemem(p, s);
	input >> NL;

	s = imagesize(180, 103, 188, 135);
	getmem(p, s);
	getimage(180, 103, 188, 135, p);
	putimage(0, 200, p, 0);
	input >> NL;

	assign(f, "v:menu.avd");
	rewrite(f);

	for (bit = 0; bit <= 3; bit ++) {
		port[0x3c4] = 2;
		port[0x3ce] = 4;
		port[0x3c5] = 1 << bit;
		port[0x3cf] = bit;
		for (gd = 0; gd <= 196; gd ++)
			for (gm = 0; gm <= 8; gm ++) {
				f << mem[0xa000 * gd * 80 + gm];
				mem[0xa000 * gd * 80 + gm] = ~ mem[0xa000 * gd * 80 + gm];
			}

		for (gd = 200; gd <= 232; gd ++) {
			f << mem[0xa000 * gd * 80];
			mem[0xa000 * gd * 80] = ~ mem[0xa000 * gd * 80];
		}
	}

	close(f);
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.