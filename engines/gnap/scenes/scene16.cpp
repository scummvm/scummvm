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
#include "gnap/scenes/scene16.h"

namespace Gnap {

Scene16::Scene16(GnapEngine *vm) : CutScene(vm) {}

int Scene16::init() {
	_sequenceIdArr[0] = 0x1F2;
	_sequenceIdArr[1] = 0x201;
	_sequenceIdArr[2] = 0x1FC;
	_sequenceIdArr[3] = 0x1F4;
	_sequenceIdArr[4] = 0x1FB;
	_sequenceIdArr[5] = 0x1F0;
	_sequenceIdArr[6] = 0x1FD;
	_sequenceIdArr[7] = 0x1FE;
	_sequenceIdArr[8] = 0x1F7;
	_sequenceIdArr[9] = 0x1F9;
	_sequenceIdArr[10] = 0x1F8;
	_sequenceIdArr[11] = 0x1F1;
	_sequenceIdArr[12] = 0x202;
	_sequenceIdArr[13] = 0x1F6;
	_sequenceIdArr[14] = 0x1F3;
	_sequenceIdArr[15] = 0x1FA;
	_sequenceIdArr[16] = 0x1FF;
	_sequenceIdArr[17] = 0x200;
	_sequenceIdArr[18] = 0x203;
	_sequenceIdArr[19] = 0x206;
	_sequenceIdArr[20] = 0x207;
	_sequenceIdArr[21] = 0x204;
	_sequenceIdArr[22] = 0x205;
	_resourceIdArr[0] = 0x1C;
	_resourceIdArr[1] = 2;
	_resourceIdArr[2] = 0x1B;
	_resourceIdArr[3] = 0;
	_resourceIdArr[4] = 0x167;
	_resourceIdArr[5] = 1;
	_resourceIdArr[6] = 0x15B;
	_resourceIdArr[7] = 0x15A;
	_resourceIdArr[8] = 0x170;
	_resourceIdArr[9] = 0x1EB;
	_resourceIdArr[10] = 0x1EC;
	_resourceIdArr[11] = 0x1BE;
	_resourceIdArr[12] = 0x1BF;
	_sequenceCountArr[0] = 4;
	_sequenceCountArr[1] = 1;
	_sequenceCountArr[2] = 1;
	_sequenceCountArr[3] = 6;
	_sequenceCountArr[4] = 1;
	_sequenceCountArr[5] = 3;
	_sequenceCountArr[6] = 1;
	_sequenceCountArr[7] = 1;
	_sequenceCountArr[8] = 1;
	_sequenceCountArr[9] = 1;
	_sequenceCountArr[10] = 1;
	_sequenceCountArr[11] = 1;
	_sequenceCountArr[12] = 1;
	_itemsCount = 13;

	return -1;
}

} // End of namespace Gnap
