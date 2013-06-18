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

/*#include "Crt.h"*/

namespace Avalanche {

typedef matrix < '\0', '\377', 0, 15, byte > fonttype;

fonttype font1;
byte fv;
char r;
file<fonttype> f;

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	assign(f, "v:avalot.fnt");
	reset(f);
	f >> font1;
	close(f);
	for (r = '\0'; r <= '\377'; r ++) {
		for (fv = 0; fv <=  3; fv ++) font1[r][fv] = (cardinal)font1[r][fv] >> 1;
		for (fv = 7; fv <=  8; fv ++) font1[r][fv] = font1[r][fv] << 1;
		for (fv = 9; fv <= 14; fv ++) font1[r][fv] = font1[r][fv] << 2;
	}
	assign(f, "v:avitalic.fnt");
	rewrite(f);
	f << font1;
	close(f);
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.