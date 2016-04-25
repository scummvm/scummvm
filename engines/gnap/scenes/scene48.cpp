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
#include "gnap/scenes/scene48.h"

namespace Gnap {

Scene48::Scene48(GnapEngine *vm) : CutScene(vm) {}

int Scene48::init() {
	_sequenceIdArr[0] = 390;
	_sequenceIdArr[1] = 391;
	_sequenceIdArr[2] = 392;
	_sequenceIdArr[3] = 393;
	_sequenceIdArr[4] = 394;
	_sequenceIdArr[5] = 395;
	_sequenceIdArr[6] = 396;
	_sequenceIdArr[7] = 397;
	_sequenceIdArr[8] = 398;
	_sequenceIdArr[9] = 399;
	_sequenceIdArr[10] = 400;
	_sequenceIdArr[11] = 401;
	_sequenceIdArr[12] = 402;
	_resourceIdArr[0] = 238;
	_resourceIdArr[1] = 42;
	_resourceIdArr[2] = 2;
	_resourceIdArr[3] = 37;
	_resourceIdArr[4] = 35;
	_resourceIdArr[5] = 38;
	_resourceIdArr[6] = 39;
	_resourceIdArr[7] = 40;
	_resourceIdArr[8] = 41;
	_resourceIdArr[9] = 36;
	_resourceIdArr[10] = 41;
	_resourceIdArr[11] = 388;
	_resourceIdArr[12] = 387;
	_sequenceCountArr[0] = 1;
	_sequenceCountArr[1] = 1;
	_sequenceCountArr[2] = 1;
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
	_itemsCount = 13;

	return -1;
}

} // End of namespace Gnap
