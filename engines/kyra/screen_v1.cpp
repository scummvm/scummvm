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
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra_v1.h"
#include "kyra/screen_v1.h"

namespace Kyra {

Screen_v1::Screen_v1(KyraEngine_v1 *vm, OSystem *system)
	: Screen(vm, system) {
	_vm = vm;
}

Screen_v1::~Screen_v1() {
}

void Screen_v1::fadeSpecialPalette(int palIndex, int startIndex, int size, int fadeTime) {
	debugC(9, kDebugLevelScreen, "Screen_v1::fadeSpecialPalette(%d, %d, %d, %d)", palIndex, startIndex, size, fadeTime);

	assert(_vm->palTable1()[palIndex]);
	assert(_currentPalette);
	uint8 tempPal[768];
	memcpy(tempPal, _currentPalette, 768);
	memcpy(&tempPal[startIndex*3], _vm->palTable1()[palIndex], size*3);
	fadePalette(tempPal, fadeTime*18);
	memcpy(&_currentPalette[startIndex*3], &tempPal[startIndex*3], size*3);
	setScreenPalette(_currentPalette);
	_system->updateScreen();
}

} // end of namespace Kyra
