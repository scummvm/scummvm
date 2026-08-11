/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TOT_DEBUG_H
#define TOT_DEBUG_H

#include "tot/tot.h"

namespace Tot {

	// Debug methods
	void drawMouseGrid(RoomFileRegister *screen);
	void drawScreenGrid(RoomFileRegister *screen);
	void drawCharacterPosition();
	void printPos(int x, int y, int screenPosX, int screenPosY, const char *label);
	void drawPos(uint x, uint y, byte color);
	void drawGrid();
	void drawRect(byte color, int x, int y, int x2, int y2);
	void drawX(int x, int y, byte color);
	void drawLine(int x, int y, int x2, int y2, byte color);
	void printNiches();

} // End of namespace Tot

#endif
