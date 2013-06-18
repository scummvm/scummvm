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

/* Avalot interrupt handler */
/*#include "Dos.h"*/

namespace Avalanche {

/*$F+*/
registers r;
void() old1b;

boolean quicko;

void new1b() {   /* interrupt; */
	;
	quicko = true;
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	;
	getintvec(0x1b, &old1b);
	setintvec(0x1b, addr(new1b()));
	quicko = false;
	do {
		;
	} while (!quicko);
	setintvec(0x1b, &old1b);
	/*  r.ah:=$02; intr($16,r);
	  writeln(r.al and 12); { Only checks Ctrl and Alt. Both on = 12. }
	 until false;*/
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.