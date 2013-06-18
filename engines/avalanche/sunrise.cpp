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
longint tskellern /*absolute $0:244*/; /* Over int $61 */

void hold() {
	do {
	} while (!(tskellern >= 1));
	tskellern = 0;
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	gd = 3;
	gm = 1;
	initgraph(gd, gm, "c:\\bp\\bgi");

	setvisualpage(1);
	setfillstyle(1, 1);
	for (gd = 1; gd <= 640; gd ++)
		bar(gd, 177 + trunc(20 * sin((real)(gd) / 39)), gd, 350);
	setfillstyle(1, 3);
	setcolor(3);
	fillellipse(320, 277, 60, 50);
	settextjustify(1, 1);
	settextstyle(0, 0, 2);
	setcolor(9);
	outtextxy(320, 50, "The sun rises over Hertfordshire...");
	settextjustify(2, 0);
	settextstyle(0, 0, 1);
	setcolor(0);
	outtextxy(635, 350, "Press any key...");

	setpalette(0, egablue);
	setpalette(1, egagreen);
	setpalette(2, egayellow);
	setpalette(3, egagreen);
	setpalette(9, egalightblue);
	setpalette(11, egalightblue);
	setvisualpage(0);

	port[0x3c4] = 2;
	port[0x3ce] = 4;
	port[0x3c5] = 1 << 1;
	port[0x3cf] = 1;

	for (gm = 227; gm >= 1; gm --) { /* <<< try running this loop the other way round! */
		move(mem[0xa000 * gm * 80 + 80], mem[0xa000 * gm * 80], 8042);
		hold();
		if (keypressed())  return 0;
	}

	for (gm = 101; gm >= 1; gm --) {
		move(mem[0xa000 * 80], mem[0xa000 * 0], gm * 80);
		hold();
		if (keypressed())  return 0;
	}
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.