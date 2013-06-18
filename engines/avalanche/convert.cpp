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
string fn1, fn2;
varying_string<30> desc;

void loadscreen(string nam) {
	byte z;
	array<1, 4, pointer> a;
	untyped_file f;
	word s;
	string check;
	;
	assign(f, nam);
	reset(f, 1);
	blockread(f, check, 41);
	blockread(f, check, 13);
	blockread(f, check, 31);
	s = imagesize(0, 0, getmaxx(), 75);
	for (z = 1; z <= 2; z ++) {
		;
		getmem(a[z], s);
		blockread(f, a[z], s);
		setactivepage(0);
		putimage(0, 15 + (z - 1) * 75, a[z], 0);
		freemem(a[z], s);
	}
	close(f);
}

void load2(string name) {
	byte a /*absolute $A000:1200*/;
	byte bit;
	untyped_file f;
	;
	assign(f, name);
	reset(f, 1);
	seek(f, 177);
	for (bit = 0; bit <= 3; bit ++) {
		;
		port[0x3c4] = 2;
		port[0x3ce] = 4;
		port[0x3c5] = 1 << bit;
		port[0x3cf] = bit;
		blockread(f, a, 12080);
	}
	close(f);
}

void save2(string name) {
	const string header =
	    string("This is a file from an Avvy game, and its contents are subject to ") +
	    "copyright." + '\15' + '\12' + '\15' + '\12' + "Have fun!" + '\32';
	byte a /*absolute $A000:1200*/;
	byte bit;
	untyped_file f;
	;
	assign(f, name);
	rewrite(f, 1);
	blockwrite(f, header[1], 146); /* really 90 */
	blockwrite(f, desc, 31);
	for (bit = 0; bit <= 3; bit ++) {
		;
		port[0x3c4] = 2;
		port[0x3ce] = 4;
		port[0x3c5] = 1 << bit;
		port[0x3cf] = bit;
		blockwrite(f, a, 12080);
	}
	close(f);
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	;
	output << "Filename?";
	input >> fn1 >> NL;
	output << "New name?";
	input >> fn2 >> NL;
	output << "Describe?";
	input >> desc >> NL;
	gd = 3;
	gm = 0;
	initgraph(gd, gm, "");
	loadscreen(fn1);
	save2(fn2);
	closegraph();
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.