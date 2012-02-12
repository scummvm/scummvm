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

#include "mortevielle/droite.h"
#include "mortevielle/graphics.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

void droite(int x, int y, int xx, int yy, int coul) {
	int step, i;
	float a, b;
	float xr, yr, xro, yro;

	xr = x;
	yr = y;
	xro = xx;
	yro = yy;
	/*writeln(le_file,'(',x:4,',',y:4,') ==> (',xx:4,',',yy:4,')');*/
	if (abs(y - yy) > abs(x - xx)) {
		a = (float)((x - xx)) / (y - yy);
		b = (yr * xro - yro * xr) / (y - yy);
		i = y;
		if (y > yy)  step = -1;
		else step = 1;
		do {
			g_vm->_screenSurface.setPixel(Common::Point(abs((int)(a * i + b)), i), coul);
			i = i + step;
		} while (!(i == yy));
		/*writeln(le_file,'X == ',a:3:3,' * Y + ',b:3:3);*/
	} else {
		a = (float)((y - yy)) / (x - xx);
		b = ((yro * xr) - (yr * xro)) / (x - xx);
		i = x;
		if (x > xx)  step = -1;
		else step = 1;
		do {
			g_vm->_screenSurface.setPixel(Common::Point(i, abs((int)(a * i + b))), coul);
			i = i + step;
		} while (!(i == xx));
		/*writeln(le_file,'Y == ',a:3:3,' * X + ',b:3:3);*/
	}
}

} // End of namespace Mortevielle
