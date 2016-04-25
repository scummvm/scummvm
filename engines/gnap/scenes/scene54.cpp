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
#include "gnap/scenes/scene54.h"

namespace Gnap {

Scene541::Scene541(GnapEngine *vm) : CutScene(vm) {}

int Scene541::init() {
	_sequenceIdArr[0] = 0x1BE;
	_sequenceIdArr[1] = 0x1BF;
	_sequenceIdArr[2] = 0x1BA;
	_sequenceIdArr[3] = 0x1BB;
	_sequenceIdArr[4] = 0x1BD;
	_sequenceIdArr[5] = 0x1BC;
	_resourceIdArr[0] = 0x3C;
	_resourceIdArr[1] = 0x43;
	_resourceIdArr[2] = 0x44;
	if (_vm->isFlag(kGFPictureTaken))
		_resourceIdArr[3] = 0x47;
	else
		_resourceIdArr[3] = 0x46;
	_resourceIdArr[4] = 0x45;
	_sequenceCountArr[0] = 1;
	_sequenceCountArr[1] = 1;
	_sequenceCountArr[2] = 1;
	_sequenceCountArr[3] = 2;
	_sequenceCountArr[4] = 1;
	_canSkip[0] = false;
	_canSkip[1] = false;
	_canSkip[2] = false;
	_canSkip[3] = false;
	_canSkip[4] = false;
	_itemsCount = 5;

	return -1;
}

Scene542::Scene542(GnapEngine *vm) : CutScene(vm) {}

int Scene542::init() {
	_sequenceIdArr[0] = 0x1C9;
	_sequenceIdArr[1] = 0x1C7;
	_sequenceIdArr[2] = 0x1CC;
	_sequenceIdArr[3] = 0x1C8;
	_sequenceIdArr[4] = 0x1CB;
	_sequenceIdArr[5] = 0x1C0;
	_sequenceIdArr[6] = 0x1CA;
	_sequenceIdArr[7] = 0x1CE;
	_sequenceIdArr[8] = 0x1CD;
	_sequenceIdArr[9] = 0x1C1;
	_sequenceIdArr[10] = 0x1C2;
	_sequenceIdArr[11] = 0x1C3;
	_sequenceIdArr[12] = 0x1C4;
	_sequenceIdArr[13] = 0x1C6;
	_sequenceIdArr[14] = 0x1C5;
	_sequenceIdArr[15] = 0x1D0;
	_sequenceIdArr[16] = 0x1D0;
	_sequenceIdArr[17] = 0x1D0;
	_resourceIdArr[0] = 0xD5;
	_resourceIdArr[1] = 0x14C;
	_resourceIdArr[2] = 0xD5;
	_resourceIdArr[3] = 0xBF;
	_resourceIdArr[4] = 0xD6;
	_resourceIdArr[5] = 0x154;
	_resourceIdArr[6] = 0x155;
	_resourceIdArr[7] = 0xB9;
	_resourceIdArr[8] = 0xBA;
	_resourceIdArr[9] = 0x17B;
	_resourceIdArr[10] = 0x17A;
	_resourceIdArr[11] = 0x17C;
	_resourceIdArr[12] = 0x17A;
	_resourceIdArr[13] = 0x1B7;
	_resourceIdArr[14] = 0x1B8;
	_resourceIdArr[15] = 0x1B9;
	_sequenceCountArr[0] = 2;
	_sequenceCountArr[1] = 1;
	_sequenceCountArr[2] = 2;
	_sequenceCountArr[3] = 1;
	_sequenceCountArr[4] = 1;
	_sequenceCountArr[5] = 1;
	_sequenceCountArr[6] = 1;
	_sequenceCountArr[7] = 1;
	_sequenceCountArr[8] = 1;
	_sequenceCountArr[9] = 1;
	_sequenceCountArr[10] = 1;
	_sequenceCountArr[11] = 1;
	_sequenceCountArr[12] = 1;
	_sequenceCountArr[13] = 1;
	_sequenceCountArr[14] = 1;
	_sequenceCountArr[15] = 1;
	_canSkip[0] = false;
	_canSkip[1] = false;
	_canSkip[2] = false;
	_canSkip[3] = false;
	_canSkip[4] = false;
	_canSkip[5] = false;
	_canSkip[6] = false;
	_canSkip[7] = false;
	_canSkip[8] = false;
	_canSkip[9] = false;
	_canSkip[10] = false;
	_canSkip[11] = false;
	_canSkip[12] = false;
	_canSkip[13] = true;
	_canSkip[14] = true;
	_canSkip[15] = false;
	_itemsCount = 16;

	return -1;
}

} // End of namespace Gnap
