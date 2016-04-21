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
	_s99_sequenceIdArr[0] = 390;
	_s99_sequenceIdArr[1] = 391;
	_s99_sequenceIdArr[2] = 392;
	_s99_sequenceIdArr[3] = 393;
	_s99_sequenceIdArr[4] = 394;
	_s99_sequenceIdArr[5] = 395;
	_s99_sequenceIdArr[6] = 396;
	_s99_sequenceIdArr[7] = 397;
	_s99_sequenceIdArr[8] = 398;
	_s99_sequenceIdArr[9] = 399;
	_s99_sequenceIdArr[10] = 400;
	_s99_sequenceIdArr[11] = 401;
	_s99_sequenceIdArr[12] = 402;
	_s99_resourceIdArr[0] = 238;
	_s99_resourceIdArr[1] = 42;
	_s99_resourceIdArr[2] = 2;
	_s99_resourceIdArr[3] = 37;
	_s99_resourceIdArr[4] = 35;
	_s99_resourceIdArr[5] = 38;
	_s99_resourceIdArr[6] = 39;
	_s99_resourceIdArr[7] = 40;
	_s99_resourceIdArr[8] = 41;
	_s99_resourceIdArr[9] = 36;
	_s99_resourceIdArr[10] = 41;
	_s99_resourceIdArr[11] = 388;
	_s99_resourceIdArr[12] = 387;
	_s99_sequenceCountArr[0] = 1;
	_s99_sequenceCountArr[1] = 1;
	_s99_sequenceCountArr[2] = 1;
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
	_s99_canSkip[0] = false;
	_s99_canSkip[1] = false;
	_s99_canSkip[2] = false;
	_s99_canSkip[3] = false;
	_s99_canSkip[4] = false;
	_s99_canSkip[5] = false;
	_s99_canSkip[6] = false;
	_s99_canSkip[7] = false;
	_s99_canSkip[8] = false;
	_s99_canSkip[9] = false;
	_s99_canSkip[10] = false;
	_s99_canSkip[11] = false;
	_s99_canSkip[12] = false;
	_s99_itemsCount = 13;

	return -1;
}

} // End of namespace Gnap
