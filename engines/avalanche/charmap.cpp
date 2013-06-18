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

/*#include "Dos.h"*/

namespace Avalanche {

struct infotype {
	matrix<128, 255, 1, 8, byte> chars;
	string data;
};
infotype table;
pointer where;
byte w, fv, ff, num;
int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	getintvec(0x1f, where);
	move(where, table, 1024);
	for (w = 128; w <= 255; w ++) {
		output << w << NL;
		for (fv = 1; fv <= 8; fv ++) {
			num = table.chars[w][fv];
			for (ff = 1; ff <= 8; ff ++) {
				if ((num & 128) == 0)  output << "  ";
				else output << "лл";
				num = num << 1;
			}
			output << NL;
		}
	}
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.