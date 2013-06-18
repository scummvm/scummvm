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

typedef matrix < '\0', '\377', 0, 15, byte > fonttype;

integer gd, gm;
fonttype font;
varying_string<79> current;
char r;

void plottext() {
	byte x, y;
	for (y = 0; y <= 7; y ++) {
		for (x = 1; x <= length(current); x ++)
			mem[0xa000 * 12880 + y * 80 + x] = font[current[x]][y];
		fillchar(mem[0xa000 * 12881 + y * 80 + x], 79 - x, '\0');
	}
}

void loadfont() {
	file<fonttype> f;
	assign(f, "c:\\thomas\\ttsmall.fnt");
	reset(f);
	f >> font;
	close(f);
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	gd = 3;
	gm = 0;
	initgraph(gd, gm, "c:\\turbo");
	loadfont();
	setfillstyle(1, 6);
	bar(0, 0, 640, 200);
	current = "";
	do {
		r = readkey();
		current = current + r;
		plottext();
	} while (!false);
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.