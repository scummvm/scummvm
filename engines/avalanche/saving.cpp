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
byte a /*absolute $A000:(15*80)*/;
byte bit;
untyped_file f;

void load(string nam) {
	byte z;
	array<1, 4, pointer> a;
	untyped_file f;
	word s;
	string check;
	assign(f, nam);
	reset(f, 1);
	blockread(f, check, 41);
	blockread(f, check, 13);
	blockread(f, check, 31);
	s = imagesize(0, 0, getmaxx(), 75);
	for (z = 1; z <= 2; z ++) {
		getmem(a[z], s);
		blockread(f, a[z], s);
		setactivepage(0);
		putimage(0, 15 + (z - 1) * 75, a[z], 0);
		freemem(a[z], s);
	}
	close(f);
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	gd = 3;
	gm = 0;
	initgraph(gd, gm, "");
	load("d:britain.avd");
	assign(f, "c:\\sleep\\test.ega");
	rewrite(f, 1);
	for (bit = 0; bit <= 3; bit ++) {
		port[0x3c4] = 2;
		port[0x3ce] = 4;
		port[0x3c5] = 1 << bit;
		port[0x3cf] = bit;
		blockwrite(f, a, 12000);
	}
	close(f);
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.