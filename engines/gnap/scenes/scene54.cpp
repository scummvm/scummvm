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

void GnapEngine::scene54_initCutscene1() {
	_s99_sequenceIdArr[0] = 0x1BE;
	_s99_sequenceIdArr[1] = 0x1BF;
	_s99_sequenceIdArr[2] = 0x1BA;
	_s99_sequenceIdArr[3] = 0x1BB;
	_s99_sequenceIdArr[4] = 0x1BD;
	_s99_sequenceIdArr[5] = 0x1BC;
	_s99_resourceIdArr[0] = 0x3C;
	_s99_resourceIdArr[1] = 0x43;
	_s99_resourceIdArr[2] = 0x44;
	if (isFlag(20))
		_s99_resourceIdArr[3] = 0x47;
	else
		_s99_resourceIdArr[3] = 0x46;
	_s99_resourceIdArr[4] = 0x45;
	_s99_sequenceCountArr[0] = 1;
	_s99_sequenceCountArr[1] = 1;
	_s99_sequenceCountArr[2] = 1;
	_s99_sequenceCountArr[3] = 2;
	_s99_sequenceCountArr[4] = 1;
	_s99_canSkip[0] = 0;
	_s99_canSkip[1] = 0;
	_s99_canSkip[2] = 0;
	_s99_canSkip[3] = 0;
	_s99_canSkip[4] = 0;
	_s99_itemsCount = 5;
}

void GnapEngine::scene54_initCutscene2() {
	_s99_sequenceIdArr[0] = 0x1C9;
	_s99_sequenceIdArr[1] = 0x1C7;
	_s99_sequenceIdArr[2] = 0x1CC;
	_s99_sequenceIdArr[3] = 0x1C8;
	_s99_sequenceIdArr[4] = 0x1CB;
	_s99_sequenceIdArr[5] = 0x1C0;
	_s99_sequenceIdArr[6] = 0x1CA;
	_s99_sequenceIdArr[7] = 0x1CE;
	_s99_sequenceIdArr[8] = 0x1CD;
	_s99_sequenceIdArr[9] = 0x1C1;
	_s99_sequenceIdArr[10] = 0x1C2;
	_s99_sequenceIdArr[11] = 0x1C3;
	_s99_sequenceIdArr[12] = 0x1C4;
	_s99_sequenceIdArr[13] = 0x1C6;
	_s99_sequenceIdArr[14] = 0x1C5;
	_s99_sequenceIdArr[15] = 0x1D0;
	_s99_sequenceIdArr[16] = 0x1D0;
	_s99_sequenceIdArr[17] = 0x1D0;
	_s99_resourceIdArr[0] = 0xD5;
	_s99_resourceIdArr[1] = 0x14C;
	_s99_resourceIdArr[2] = 0xD5;
	_s99_resourceIdArr[3] = 0xBF;
	_s99_resourceIdArr[4] = 0xD6;
	_s99_resourceIdArr[5] = 0x154;
	_s99_resourceIdArr[6] = 0x155;
	_s99_resourceIdArr[7] = 0xB9;
	_s99_resourceIdArr[8] = 0xBA;
	_s99_resourceIdArr[9] = 0x17B;
	_s99_resourceIdArr[10] = 0x17A;
	_s99_resourceIdArr[11] = 0x17C;
	_s99_resourceIdArr[12] = 0x17A;
	_s99_resourceIdArr[13] = 0x1B7;
	_s99_resourceIdArr[14] = 0x1B8;
	_s99_resourceIdArr[15] = 0x1B9;
	_s99_sequenceCountArr[0] = 2;
	_s99_sequenceCountArr[1] = 1;
	_s99_sequenceCountArr[2] = 2;
	_s99_sequenceCountArr[3] = 1;
	_s99_sequenceCountArr[4] = 1;
	_s99_sequenceCountArr[5] = 1;
	_s99_sequenceCountArr[6] = 1;
	_s99_sequenceCountArr[7] = 1;
	_s99_sequenceCountArr[8] = 1;
	_s99_sequenceCountArr[9] = 1;
	_s99_sequenceCountArr[10] = 1;
	_s99_sequenceCountArr[11] = 1;
	_s99_sequenceCountArr[12] = 1;
	_s99_sequenceCountArr[13] = 1;
	_s99_sequenceCountArr[14] = 1;
	_s99_sequenceCountArr[15] = 1;
	_s99_canSkip[0] = 0;
	_s99_canSkip[1] = 0;
	_s99_canSkip[2] = 0;
	_s99_canSkip[3] = 0;
	_s99_canSkip[4] = 0;
	_s99_canSkip[5] = 0;
	_s99_canSkip[6] = 0;
	_s99_canSkip[7] = 0;
	_s99_canSkip[8] = 0;
	_s99_canSkip[9] = 0;
	_s99_canSkip[10] = 0;
	_s99_canSkip[11] = 0;
	_s99_canSkip[12] = 0;
	_s99_canSkip[13] = 1;
	_s99_canSkip[14] = 1;
	_s99_canSkip[15] = 0;
	_s99_itemsCount = 16;
}

} // End of namespace Gnap
