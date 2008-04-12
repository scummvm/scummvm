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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "kyra/screen_v3.h"

#include "kyra/kyra_v3.h"

namespace Kyra {

Screen_v3::Screen_v3(KyraEngine_v3 *vm, OSystem *system) : ScreenEx(vm, system) {
}

Screen_v3::~Screen_v3() {
}

void Screen_v3::setScreenDim(int dim) {
	debugC(9, kDebugLevelScreen, "Screen_v3::setScreenDim(%d)", dim);
	assert(dim < _screenDimTableCount);
	_curDim = &_screenDimTable[dim];
}

const ScreenDim *Screen_v3::getScreenDim(int dim) {
	debugC(9, kDebugLevelScreen, "Screen_v3::getScreenDim(%d)", dim);
	assert(dim < _screenDimTableCount);
	return &_screenDimTable[dim];
}

} // end of namespace Kyra