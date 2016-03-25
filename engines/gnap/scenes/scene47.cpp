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
 */

#include "gnap/gnap.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"

namespace Gnap {

void GnapEngine::scene47_initCutscene1() {
	_s99_sequenceIdArr[0] = 0x301;
	_s99_sequenceIdArr[1] = 0x305;
	_s99_sequenceIdArr[2] = 0x302;
	_s99_sequenceIdArr[3] = 0x304;
	_s99_sequenceIdArr[4] = 0x300;
	_s99_resourceIdArr[0] = 3;
	_s99_resourceIdArr[1] = 0;
	_s99_resourceIdArr[2] = 1;
	_s99_resourceIdArr[3] = 0;
	_s99_resourceIdArr[4] = 2;
	_s99_sequenceCountArr[0] = 1;
	_s99_sequenceCountArr[1] = 1;
	_s99_sequenceCountArr[2] = 1;
	_s99_sequenceCountArr[3] = 1;
	_s99_sequenceCountArr[4] = 1;
	_s99_canSkip[0] = false;
	_s99_canSkip[1] = false;
	_s99_canSkip[2] = false;
	_s99_canSkip[3] = false;
	_s99_canSkip[4] = false;
	_s99_itemsCount = 5;
}

void GnapEngine::scene47_initCutscene2() {
	_s99_sequenceIdArr[0] = 0x306;
	_s99_sequenceIdArr[1] = 0x309;
	_s99_sequenceIdArr[2] = 0x307;
	_s99_sequenceIdArr[3] = 0x308;
	_s99_sequenceIdArr[4] = 0x30A;
	_s99_resourceIdArr[0] = 0x8E;
	_s99_resourceIdArr[1] = 0x90;
	_s99_resourceIdArr[2] = 0x8F;
	_s99_resourceIdArr[3] = 0x91;
	_s99_sequenceCountArr[0] = 2;
	_s99_sequenceCountArr[1] = 1;
	_s99_sequenceCountArr[2] = 1;
	_s99_sequenceCountArr[3] = 1;
	_s99_canSkip[0] = false;
	_s99_canSkip[1] = false;
	_s99_canSkip[2] = false;
	_s99_canSkip[3] = false;
	_s99_itemsCount = 4;
}

void GnapEngine::scene47_initCutscene3() {
	_s99_sequenceIdArr[0] = 0x320;
	_s99_sequenceIdArr[1] = 0x321;
	_s99_resourceIdArr[0] = 0x142;
	_s99_resourceIdArr[1] = 0x143;
	_s99_sequenceCountArr[0] = 1;
	_s99_sequenceCountArr[1] = 1;
	_s99_canSkip[0] = false;
	_s99_canSkip[1] = false;
	_s99_itemsCount = 2;
}

void GnapEngine::scene47_initCutscene4() {
	_s99_sequenceIdArr[0] = 0x30C;
	_s99_sequenceIdArr[1] = 0x30D;
	_s99_sequenceIdArr[2] = 0x30B;
	_s99_resourceIdArr[0] = 0x142;
	_s99_resourceIdArr[1] = 0x141;
	_s99_resourceIdArr[2] = 0x177;
	_s99_sequenceCountArr[0] = 1;
	_s99_sequenceCountArr[1] = 1;
	_s99_sequenceCountArr[2] = 1;
	_s99_canSkip[0] = false;
	_s99_canSkip[1] = false;
	_s99_canSkip[2] = false;
	_s99_itemsCount = 3;
}

void GnapEngine::scene47_initCutscene5() {
	_s99_sequenceIdArr[0] = 0x30E;
	_s99_sequenceIdArr[1] = 0x30F;
	_s99_sequenceIdArr[2] = 0x310;
	_s99_sequenceIdArr[3] = 0x311;
	_s99_resourceIdArr[0] = 0x206;
	_s99_resourceIdArr[1] = 0x207;
	_s99_sequenceCountArr[0] = 3;
	_s99_sequenceCountArr[1] = 1;
	_s99_canSkip[0] = false;
	_s99_canSkip[1] = false;
	_s99_itemsCount = 2;
}

void GnapEngine::scene47_initCutscene6() {
	_s99_sequenceIdArr[0] = 0x31E;
	_s99_sequenceIdArr[1] = 0x31F;
	_s99_resourceIdArr[0] = 0x2FA;
	_s99_sequenceCountArr[0] = 2;
	_s99_canSkip[0] = false;
	_s99_itemsCount = 1;
}

void GnapEngine::scene47_initCutscene7() {
	int v0, v4, v2, v3;
	
	_s99_sequenceIdArr[0] = 0x316;
	_s99_sequenceIdArr[1] = 0x31A;
	_s99_sequenceIdArr[2] = 0x314;
	_s99_sequenceIdArr[3] = 0x31B;
	int v1 = 4;
	if (!isFlag(3)) {
		_s99_sequenceIdArr[4] = 0x31C;
		v1 = 5;
	}
	if (!isFlag(26))
		_s99_sequenceIdArr[v1++] = 0x31D;
	v4 = v1;
	_s99_sequenceIdArr[v1] = 0x319;
	v0 = v1 + 1;
	v3 = v0;
	_s99_sequenceIdArr[v0++] = 0x317;
	_s99_sequenceIdArr[v0++] = 0x312;
	_s99_sequenceIdArr[v0] = 0x31A;
	v2 = v0 + 1;
	if (!isFlag(3))
		_s99_sequenceIdArr[v2++] = 0x31C;
	if (!isFlag(26))
		_s99_sequenceIdArr[v2++] = 0x31D;
	_s99_sequenceIdArr[v2] = 0x313;
	_s99_sequenceIdArr[v2 + 1] = 0x315;
	_s99_resourceIdArr[0] = 0x2B8;
	_s99_resourceIdArr[1] = 0x20C;
	_s99_resourceIdArr[2] = 0x2B8;
	_s99_resourceIdArr[3] = 0x20B;
	_s99_resourceIdArr[4] = 0x20B;
	_s99_sequenceCountArr[0] = v4;
	_s99_sequenceCountArr[1] = 1;
	_s99_sequenceCountArr[2] = v2 - v3;
	_s99_sequenceCountArr[3] = 1;
	_s99_sequenceCountArr[4] = 1;
	_s99_canSkip[0] = false;
	_s99_canSkip[1] = false;
	_s99_canSkip[2] = false;
	_s99_canSkip[3] = false;
	_s99_canSkip[4] = false;
	_s99_itemsCount = 5;
}

} // End of namespace Gnap
