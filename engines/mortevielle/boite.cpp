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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#include "mortevielle/boite.h"
#include "mortevielle/mouse.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

void boite(int x, int y, int dx, int dy, int coul) {
	int i;
	int xi, yi, xo, yo;

	/* debug('boite'); */
	hide_mouse();
	if (res == 1) {
		x = (cardinal)x >> 1;
		dx = (cardinal)dx >> 1;
	}
	xi = x;
	yi = y;
	xo = x;
	yo = y;
	for (i = 0; i <= dx + dy; i ++) {
		putpix(gd, xi, yi, coul);
		if (xi == x + dx)  {
			if (gd != cga)  putpix(gd, pred(int, xi), yi, coul);
			yi = succ(int, yi);
		} else xi = succ(int, xi);
		putpix(gd, xo, yo, coul);
		if (yo == y + dy)  xo = succ(int, xo);
		else {
			if (gd != cga)  putpix(gd, succ(int, xo), yo, coul);
			yo = succ(int, yo);
		}
	}
	show_mouse();
}

} // End of namespace Mortevielle
