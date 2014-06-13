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
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#ifndef MORTEVIELLE_MOUSE_H
#define MORTEVIELLE_MOUSE_H

#include "common/rect.h"

namespace Mortevielle {
class MortevielleEngine;

class MouseHandler {
private:
	MortevielleEngine *_vm;
public:
	Common::Point _pos;
	MouseHandler(MortevielleEngine *vm);

	void initMouse();
	void hideMouse();
	void showMouse();
	void setMousePosition(Common::Point newPos);
	void getMousePosition(int &x, int &y, bool &click);
	void moveMouse(bool &funct, char &key);
	bool isMouseIn(Common::Rect r);
};

} // End of namespace Mortevielle
#endif
