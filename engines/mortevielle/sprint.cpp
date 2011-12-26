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

#include "common/str.h"

namespace MORTEVIELLE {

void fill_box(int x, int y, int dx) {
	box(15, gd, pred(int, x) << 3, pred(int, y) << 3, pred(int, x + dx) << 3, y << 3, 255);
}

void clear_box(int x, int y, int dx) {
	box(0, gd, pred(int, x) << 3, pred(int, y) << 3, pred(int, x + dx) << 3, y << 3, 255);
}

void writeg(Common::String l, int c)

{
	int i, x, xo, yo;
	int cecr;
	bool t;

	/*  debug('writeg : '+l);*/

	if (l == "")  return;
	hide_mouse();
	xo = xwhere;
	yo = ywhere;
	if (res == 2)  i = 6;
	else i = 10;
	x = xo + i * length(l);
	switch (c) {
	case 1:
	case 3  : {
		cecr = 0;
		box(15, gd, xo, yo, x, yo + 7, 255);
	}
	break;
	case 4 : {
		cecr = 0;
	}
	break;
	case 5 : {
		cecr = 15;
	}
	break;
	case 0:
	case 2 : {
		cecr = 15;
		box(0, gd, xo, yo, x, yo + 7, 255);
	}
	break;
	}
	xo = xo + 1;
	yo = yo + 1;
	for (x = 1; x <= length(l); x ++) {
		affcar(gd, xo, yo, cecr, ord(l[x]));
		xo = xo + i;
	}
	show_mouse();
}

} // End of namespace MORTEVIELLE
