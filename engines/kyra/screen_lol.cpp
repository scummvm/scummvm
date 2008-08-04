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

#include "kyra/screen_lol.h"
#include "kyra/lol.h"

namespace Kyra {

Screen_LoL::Screen_LoL(LoLEngine *vm, OSystem *system) : Screen_v2(vm, system), _vm(vm) {
}

void Screen_LoL::setScreenDim(int dim) {
	debugC(9, kDebugLevelScreen, "Screen_LoL::setScreenDim(%d)", dim);
	assert(dim < _screenDimTableCount);
	_curDim = &_screenDimTable[dim];
}

const ScreenDim *Screen_LoL::getScreenDim(int dim) {
	debugC(9, kDebugLevelScreen, "Screen_LoL::getScreenDim(%d)", dim);
	assert(dim < _screenDimTableCount);
	return &_screenDimTable[dim];
}

void Screen_LoL::fprintStringIntro(const char *format, int x, int y, uint8 c1, uint8 c2, uint8 c3, uint16 flags, ...) {
	debugC(9, kDebugLevelScreen, "Screen_LoL::fprintStringIntro('%s', %d, %d, %d, %d, %d, %d, ...)", format, x, y, c1, c2, c3, flags);
	char buffer[400];

	va_list args;
	va_start(args, flags);
	vsprintf(buffer, format, args);	
	va_end(args);
	
	if ((flags & 0x0F00) == 0x100)
		x -= getTextWidth(buffer) >> 1;
	if ((flags & 0x0F00) == 0x200)
		x -= getTextWidth(buffer);

	if ((flags & 0x00F0) == 0x20) {
		printText(buffer, x-1, y, c3, c2);
		printText(buffer, x, y+1, c3, c2);
	}

	printText(buffer, x, y, c1, c2);
}

} // end of namespace Kyra

