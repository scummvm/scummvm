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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_MOUSE_H
#define HUGO_MOUSE_H
namespace Hugo {

class MouseHandler {
public:
	MouseHandler(HugoEngine &vm);

	void mouseHandler();

private:
	HugoEngine &_vm;

	void  cursorText(char *buffer, int16 cx, int16 cy, uif_t fontId, int16 color);
	int16 findExit(int16 cx, int16 cy);
	void  processRightClick(int16 objId, int16 cx, int16 cy);
	void  processLeftClick(int16 objId, int16 cx, int16 cy);
};

} // end of namespace Hugo
#endif //HUGO_MOUSE_H
