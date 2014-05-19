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
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef CGE2_EVENTS_H
#define CGE2_EVENTS_H

#include "common/events.h"
#include "cge2/talk.h"
#include "cge2/vga13h.h"

namespace CGE2 {

/*----------------- MOUSE interface -----------------*/

class Mouse : public Sprite {
public:
	Sprite *_hold;
	bool _active;
	int _hx;
	int _hy;
	bool _exist;
	int _buttons;
	Sprite *_busy;
	Mouse(CGE2Engine *vm);
	~Mouse();
	void on();
	void off();
	void newMouse(Common::Event &event);
private:
	CGE2Engine *_vm;
};

} // End of namespace CGE

#endif // #define CGE2_EVENTS_H
