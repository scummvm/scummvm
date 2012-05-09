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

#include "lilliput/lilliput.h"
#include "lilliput/sound.h"

#include "common/debug.h"

namespace Lilliput {

LilliputSound::LilliputSound(LilliputEngine *vm) : _vm(vm) {
}

LilliputSound::~LilliputSound() {
}

// Used during initialisation
void LilliputSound::contentFct0() {
	debugC(1, kDebugSound, "contentFct0()");
}

void LilliputSound::contentFct1() {
	debugC(1, kDebugSound, "contentFct1()");
}

void LilliputSound::contentFct2(int var1, Common::Point var2, Common::Point var3, Common::Point var4) {
	debugC(1, kDebugSound, "contentFct2(%d, %d - %d, %d - %d, %d - %d)", var1, var2.x, var2.y, var3.x, var3.y, var4.x, var4.y);
}

void LilliputSound::contentFct3() {
	debugC(1, kDebugSound, "contentFct3()");
}

void LilliputSound::contentFct4() {
	debugC(1, kDebugSound, "contentFct4()");
}

void LilliputSound::contentFct5() {
	debugC(1, kDebugSound, "contentFct5()");
}

void LilliputSound::contentFct6() {
	debugC(1, kDebugSound, "contentFct6()");
}

} // End of namespace
