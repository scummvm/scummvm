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
#include "palette.h"
#include "assert.h"

Palette::Palette() {
}

Palette::Palette(unsigned char * ptr) {
	for(int i = 0; i < 256; i++) {
		_colors[i] = Color(ptr[3 * i + 0], ptr[3 * i + 1], ptr[3 * i + 2]);
	}
}

Palette::Palette(const Palette & p) {
	for(int i = 0; i < 256; i++) {
		_colors[i] = p._colors[i];
	}
}

const Color & Palette::operator[](int a) const {
	assert(a >= 0 && a < 256);
	return _colors[a];
}

Color & Palette::operator[](int a) {
	assert(a >= 0 && a < 256);
	return _colors[a];
}

Palette & Palette::operator=(const Palette & p) {
	for(int i = 0; i < 256; i++) {
		_colors[i] = p._colors[i];
	}
	return *this;
}
