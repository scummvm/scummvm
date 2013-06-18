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

namespace Avalanche {

enum flavourtype {ch_ega, ch_bgi, ch_natural, ch_two, ch_one, last_flavourtype};

struct chunkblocktype {
	union {
		struct {
			flavourtype flavour;
			integer x, y;
			integer xl, yl;
			longint size;
		} s_true;
		array<1, 14, byte> all;
	};
};


byte screennum;
untyped_file f;

string strf(longint x) {
	string q;
	string strf_result;
	str(x, q);
	strf_result = q;
	return strf_result;
}

void load() {
	byte a /*absolute $A000:1200*/;
	byte bit;
	reset(f, 1);
	seek(f, 177);
	for (bit = 0; bit <= 3; bit ++) {
		port[0x3c4] = 2;
		port[0x3ce] = 4;
		port[0x3c5] = 1 << bit;
		port[0x3cf] = bit;
		blockread(f, a, 12080);
	}
	close(f);
	bit = getpixel(0, 0);
}

void init() {
	integer gd, gm;
	output << "*** Tempo file creater ***" << NL;
	output << "Enter place*.avd number:";
	input >> screennum >> NL;
	gd = 3;
	gm = 0;
	initgraph(gd, gm, "c:\\bp\\bgi");
	assign(f, string("place") + strf(screennum) + ".avd");
	load();
	setactivepage(1);
	setcolor(10);
	outtextxy(0, 150, "CHUNK FILE: please don't change these codes! ->");
	setactivepage(0);
}

void choose() {
	integer x1, y1, xl, yl;

}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	init();
	choose();
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.