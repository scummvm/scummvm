/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include <stdafx.h>
#include "color.h"

#define UPDATE_COLOR(c, inc) (((int)((c)) << 7) + (c) + (inc)) >> 7
#define CHECK_BOUNDS(c) (((c) > 255) ? 255 : (((c) < 0) ? 0 : (c)))

void Color::delta(short * ptr) {
	// This is a very specific method for XPALs.
	int t;
	t = UPDATE_COLOR(_r, ptr[0]);
	_r = CHECK_BOUNDS(t);
	t = UPDATE_COLOR(_g, ptr[1]);
	_g = CHECK_BOUNDS(t);
	t = UPDATE_COLOR(_b, ptr[2]);
	_b = CHECK_BOUNDS(t);
}
