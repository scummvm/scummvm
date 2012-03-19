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

#ifndef MORTEVIELLE_MOUSE_H
#define MORTEVIELLE_MOUSE_H

#include "mortevielle/var_mor.h"

namespace Mortevielle {

class MouseHandler {
protected:
	int s_s[12][6];
public:
	bool int_m;

	int m_show;
	int x_s;
	int y_s;
	int p_o_s;
	int mouse_shwn;

	void initMouse();
	void hideMouse();
	void showMouse();
	void setMousePos(int x, int y);
	void getMousePos_(int &x, int &y, int &c);
	void moveMouse(bool &funct, char &key);
	bool isMouseIn(rectangle r);
};

} // End of namespace Mortevielle
#endif
