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

#ifndef CGE_GAME_H
#define CGE_GAME_H

#include "cge/vga13h.h"

namespace CGE {

uint8 *glass(Dac *pal, uint8 r, uint8 g, uint8 b);

class Fly : public Sprite {
	static const int _l;
	static const int _t;
	static const int _r;
	static const int _b;
public:
	int _tx, _ty;
	Fly(CGEEngine *vm, Bitmap **shpl);
	void tick();
private:
	CGEEngine *_vm;
};

} // End of namespace CGE

#endif
