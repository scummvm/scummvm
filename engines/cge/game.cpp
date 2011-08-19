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
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge/game.h"
#include "cge/events.h"

namespace CGE {

uint8 *glass(Dac *pal, uint8 r, uint8 g, uint8 b) {
	uint8 *x = (uint8 *)malloc(256);
	if (x) {
		uint16 i;
		for (i = 0; i < 256; i++) {
			x[i] = closest(pal, mkDac(((uint16)(pal[i]._r) * r) / 255,
			                          ((uint16)(pal[i]._g) * g) / 255,
			                          ((uint16)(pal[i]._b) * b) / 255));
		}
	}
	return x;
}

const int Fly::_l = 20,
    Fly::_t = 40,
    Fly::_r = 110,
    Fly::_b = 100;

Fly::Fly(CGEEngine *vm, Bitmap **shpl)
	: Sprite(vm, shpl), _tx(0), _ty(0), _vm(vm) {
	step(newRandom(2));
	gotoxy(_l + newRandom(_r - _l - _w), _t + newRandom(_b - _t - _h));
}

void Fly::tick() {
	step();
	if (_flags._kept)
		return;
	if (newRandom(10) < 1) {
		_tx = newRandom(3) - 1;
		_ty = newRandom(3) - 1;
	}
	if (_x + _tx < _l || _x + _tx + _w > _r)
		_tx = -_tx;
	if (_y + _ty < _t || _y + _ty + _h > _b)
		_ty = -_ty;
	gotoxy(_x + _tx, _y + _ty);
}

} // End of namespace CGE
