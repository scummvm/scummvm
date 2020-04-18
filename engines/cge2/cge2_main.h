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

#ifndef CGE2_MAIN_H
#define CGE2_MAIN_H

#include "cge2/events.h"

namespace CGE2 {

#define kShowScummVMVersion 15

class System : public Sprite {
public:
	int _blinkCounter;
	Sprite *_blinkSprite;

	System(CGE2Engine *vm);

	void touch(uint16 mask, V2D pos, Common::KeyCode keyCode) override;
	void tick() override;
private:
	CGE2Engine *_vm;
};

} // End of namespace CGE2

#endif // CGE2_MAIN_H
