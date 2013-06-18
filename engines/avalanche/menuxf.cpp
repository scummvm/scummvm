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

integer gd, gm;

void load() {   /* Load2, actually */
	byte a0 /*absolute $A000:800*/;
	byte bit;
	untyped_file f;

	assign(f, "maintemp.avd");
	reset(f, 1);
	/* Compression method byte follows this... */
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

void save() {
	byte bit;
	untyped_file f;

	assign(f, "v:mainmenu.avd");
	rewrite(f, 1);
	for (bit = 0; bit <= 3; bit ++) {
		port[0x3c4] = 2;
		port[0x3ce] = 4;
		port[0x3c5] = 1 << bit;
		port[0x3cf] = bit;
		blockwrite(f, mem[0xa000 * 48 * 80], 59 * 80);
	}
	close(f);
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	gd = 3;
	gm = 1;
	initgraph(gd, gm, "");
	load(); /* Between 48 and 107. */

	save();
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.