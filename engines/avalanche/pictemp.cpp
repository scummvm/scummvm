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

const integer picsize = 966;

integer gd, gm;
untyped_file f;
pointer p;

void save() {
	varying_string<40> adf;
	untyped_file f;
	byte z;
	array<1, 2, pointer> c;
	string nam, screenname;
	const string header =
	    string("This is a file from an Avvy game, and its contents are subject to ") +
	    "copyright." + '\15' + '\12' + '\15' + '\12' + "Have fun!" + '\32';
	byte a /*absolute $A000:1200i*/;
	byte bit;
	;
	nam = "d:thingtmp.avd";
	screenname = "Temp.";
	assign(f, nam);

	assign(f, nam);
	rewrite(f, 1);
	blockwrite(f, header[1], 146);
	blockwrite(f, screenname, 31);
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
	gd = 3;
	gm = 0;
	initgraph(gd, gm, "c:\\bp\\bgi");

	assign(f, "thinks.avd");
	getmem(p, picsize);
	reset(f, 1);
	seek(f, 65);
	gd = 10;
	gm = 20;

	while (gm < 120) {
		;
		if (! eof(f))
			blockread(f, p, picsize);
		putimage(gd, gm, p, 0);
		gd += 70;

		if (gd == 640) {
			;
			gd = 10;
			gm += 40;
		}

	}

	close(f);
	freemem(p, picsize);
	save();
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.