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

void GnapEngine::scene16_initCutscene() {
	_s99_sequenceIdArr[0] = 0x1F2;
	_s99_sequenceIdArr[1] = 0x201;
	_s99_sequenceIdArr[2] = 0x1FC;
	_s99_sequenceIdArr[3] = 0x1F4;
	_s99_sequenceIdArr[4] = 0x1FB;
	_s99_sequenceIdArr[5] = 0x1F0;
	_s99_sequenceIdArr[6] = 0x1FD;
	_s99_sequenceIdArr[7] = 0x1FE;
	_s99_sequenceIdArr[8] = 0x1F7;
	_s99_sequenceIdArr[9] = 0x1F9;
	_s99_sequenceIdArr[10] = 0x1F8;
	_s99_sequenceIdArr[11] = 0x1F1;
	_s99_sequenceIdArr[12] = 0x202;
	_s99_sequenceIdArr[13] = 0x1F6;
	_s99_sequenceIdArr[14] = 0x1F3;
	_s99_sequenceIdArr[15] = 0x1FA;
	_s99_sequenceIdArr[16] = 0x1FF;
	_s99_sequenceIdArr[17] = 0x200;
	_s99_sequenceIdArr[18] = 0x203;
	_s99_sequenceIdArr[19] = 0x206;
	_s99_sequenceIdArr[20] = 0x207;
	_s99_sequenceIdArr[21] = 0x204;
	_s99_sequenceIdArr[22] = 0x205;
	_s99_resourceIdArr[0] = 0x1C;
	_s99_resourceIdArr[1] = 2;
	_s99_resourceIdArr[2] = 0x1B;
	_s99_resourceIdArr[3] = 0;
	_s99_resourceIdArr[4] = 0x167;
	_s99_resourceIdArr[5] = 1;
	_s99_resourceIdArr[6] = 0x15B;
	_s99_resourceIdArr[7] = 0x15A;
	_s99_resourceIdArr[8] = 0x170;
	_s99_resourceIdArr[9] = 0x1EB;
	_s99_resourceIdArr[10] = 0x1EC;
	_s99_resourceIdArr[11] = 0x1BE;
	_s99_resourceIdArr[12] = 0x1BF;
	_s99_sequenceCountArr[0] = 4;
	_s99_sequenceCountArr[1] = 1;
	_s99_sequenceCountArr[2] = 1;
	_s99_sequenceCountArr[3] = 6;
	_s99_sequenceCountArr[4] = 1;
	_s99_sequenceCountArr[5] = 3;
	_s99_sequenceCountArr[6] = 1;
	_s99_sequenceCountArr[7] = 1;
	_s99_sequenceCountArr[8] = 1;
	_s99_sequenceCountArr[9] = 1;
	_s99_sequenceCountArr[10] = 1;
	_s99_sequenceCountArr[11] = 1;
	_s99_sequenceCountArr[12] = 1;
	_s99_itemsCount = 13;
}

} // End of namespace Gnap
