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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#ifndef CGE2_SPARE_H
#define CGE2_SPARE_H

#include "cge2/vga13h.h"

namespace CGE2 {

class Spare {
	CGE2Engine *_vm;
	Common::Array<Sprite*> _container;
public:
	Spare(CGE2Engine *vm) : _vm(vm) {}
	~Spare() { clear(); }
	void store(Sprite *spr);
	Sprite *locate(int ref);
	Sprite *take(int ref);
	void takeScene(int cav);
	void update(Sprite *spr);
	void dispose(Sprite *spr);
	void dispose(int ref);
	void dispose();
	void sync(Common::Serializer &s);
	uint16 count() { return _container.size(); }
	void clear();
};

} // End of namespace CGE2

#endif // CGE2_SPARE_H
