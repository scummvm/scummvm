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
/*#include "Dos.h"*/

namespace Avalanche {

integer gd, gm;
string filename;

untyped_file f;
byte bit;
byte a /*absolute $A000:800*/;
searchrec r;

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	;
	gd = 3;
	gm = 0;
	initgraph(gd, gm, "c:\\bp\\bgi");

	for (gd = 0; gd <= 14; gd ++)
		for (gm = 0; gm <= 11; gm ++) {
			;
			setfillstyle(gm, gd + 1);
			bar(gd * 43, gm * 12 + 10, 42 + gd * 43, gm * 12 + 21);
		}

	output << NL;
	output << NL;
	output << NL;
	output << NL;

	output << "Thorsoft testcard." << NL;
	output << NL;
	output << "Room number? ";
	input >> filename >> NL;

	findfirst(string("place") + filename + ".avd", anyfile, r);
	if (doserror == 0) {
		;
		output << "*** ALREADY EXISTS! CANCELLED! ***" << NL;
		input >> NL;
		exit(0);
	}

	output << NL;
	output << "*** Until this room is drawn, this screen is standing in for it. ***" << NL;
	output << NL;
	output << "Any other comments? ";
	input >> NL;

	assign(f, string("place") + filename + ".avd");
	rewrite(f, 1);
	blockwrite(f, gd, 177); /* just anything */
	for (bit = 0; bit <= 3; bit ++) {
		;
		port[0x3c4] = 2;
		port[0x3ce] = 4;
		port[0x3c5] = 1 << bit;
		port[0x3cf] = bit;
		blockwrite(f, a, 12080);
	}
	close(f);
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.