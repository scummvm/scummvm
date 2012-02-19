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

namespace Mortevielle {

extern bool int_m;

extern int m_show,
        x_s,
        y_s,
        p_o_s,
        mouse_shwn;

void initMouse();
//extern void change_mouse(int t);
//extern void dessine_souris(int xf, int yf);
extern void hideMouse();
extern void showMouse();
extern void setMousePos(int x, int y);
extern void getMousePos(int &x, int &y, int &c);
extern void moveMouse(bool &funct, char &key);

} // End of namespace Mortevielle
#endif
