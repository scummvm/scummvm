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
#include "gnap/scenes/scene47.h"

namespace Gnap {

Scene471::Scene471(GnapEngine *vm) : CutScene(vm) {}

int Scene471::init() {
	_sequenceIdArr[0] = 0x301;
	_sequenceIdArr[1] = 0x305;
	_sequenceIdArr[2] = 0x302;
	_sequenceIdArr[3] = 0x304;
	_sequenceIdArr[4] = 0x300;
	_resourceIdArr[0] = 3;
	_resourceIdArr[1] = 0;
	_resourceIdArr[2] = 1;
	_resourceIdArr[3] = 0;
	_resourceIdArr[4] = 2;
	_sequenceCountArr[0] = 1;
	_sequenceCountArr[1] = 1;
	_sequenceCountArr[2] = 1;
	_sequenceCountArr[3] = 1;
	_sequenceCountArr[4] = 1;
	_canSkip[0] = false;
	_canSkip[1] = false;
	_canSkip[2] = false;
	_canSkip[3] = false;
	_canSkip[4] = false;
	_itemsCount = 5;

	return -1;
}

Scene472::Scene472(GnapEngine *vm) : CutScene(vm) {}

int Scene472::init() {
	_sequenceIdArr[0] = 0x306;
	_sequenceIdArr[1] = 0x309;
	_sequenceIdArr[2] = 0x307;
	_sequenceIdArr[3] = 0x308;
	_sequenceIdArr[4] = 0x30A;
	_resourceIdArr[0] = 0x8E;
	_resourceIdArr[1] = 0x90;
	_resourceIdArr[2] = 0x8F;
	_resourceIdArr[3] = 0x91;
	_sequenceCountArr[0] = 2;
	_sequenceCountArr[1] = 1;
	_sequenceCountArr[2] = 1;
	_sequenceCountArr[3] = 1;
	_canSkip[0] = false;
	_canSkip[1] = false;
	_canSkip[2] = false;
	_canSkip[3] = false;
	_itemsCount = 4;

	return -1;
}

Scene473::Scene473(GnapEngine *vm) : CutScene(vm) {}

int Scene473::init() {
	_sequenceIdArr[0] = 0x320;
	_sequenceIdArr[1] = 0x321;
	_resourceIdArr[0] = 0x142;
	_resourceIdArr[1] = 0x143;
	_sequenceCountArr[0] = 1;
	_sequenceCountArr[1] = 1;
	_canSkip[0] = false;
	_canSkip[1] = false;
	_itemsCount = 2;

	return -1;
}

Scene474::Scene474(GnapEngine *vm) : CutScene(vm) {}

int Scene474::init() {
	_sequenceIdArr[0] = 0x30C;
	_sequenceIdArr[1] = 0x30D;
	_sequenceIdArr[2] = 0x30B;
	_resourceIdArr[0] = 0x142;
	_resourceIdArr[1] = 0x141;
	_resourceIdArr[2] = 0x177;
	_sequenceCountArr[0] = 1;
	_sequenceCountArr[1] = 1;
	_sequenceCountArr[2] = 1;
	_canSkip[0] = false;
	_canSkip[1] = false;
	_canSkip[2] = false;
	_itemsCount = 3;

	return -1;
}

Scene475::Scene475(GnapEngine *vm) : CutScene(vm) {}

int Scene475::init() {
	_sequenceIdArr[0] = 0x30E;
	_sequenceIdArr[1] = 0x30F;
	_sequenceIdArr[2] = 0x310;
	_sequenceIdArr[3] = 0x311;
	_resourceIdArr[0] = 0x206;
	_resourceIdArr[1] = 0x207;
	_sequenceCountArr[0] = 3;
	_sequenceCountArr[1] = 1;
	_canSkip[0] = false;
	_canSkip[1] = false;
	_itemsCount = 2;

	return -1;
}

Scene476::Scene476(GnapEngine *vm) : CutScene(vm) {}

int Scene476::init() {
	_sequenceIdArr[0] = 0x31E;
	_sequenceIdArr[1] = 0x31F;
	_resourceIdArr[0] = 0x2FA;
	_sequenceCountArr[0] = 2;
	_canSkip[0] = false;
	_itemsCount = 1;

	return -1;
}

Scene477::Scene477(GnapEngine *vm) : CutScene(vm) {}

int Scene477::init() {
	int v0, v4, v2, v3;
	
	_sequenceIdArr[0] = 0x316;
	_sequenceIdArr[1] = 0x31A;
	_sequenceIdArr[2] = 0x314;
	_sequenceIdArr[3] = 0x31B;
	int v1 = 4;
	if (!_vm->isFlag(kGFTwigTaken)) {
		_sequenceIdArr[4] = 0x31C;
		v1 = 5;
	}
	if (!_vm->isFlag(kGFPlatypusTalkingToAssistant))
		_sequenceIdArr[v1++] = 0x31D;
	v4 = v1;
	_sequenceIdArr[v1] = 0x319;
	v0 = v1 + 1;
	v3 = v0;
	_sequenceIdArr[v0++] = 0x317;
	_sequenceIdArr[v0++] = 0x312;
	_sequenceIdArr[v0] = 0x31A;
	v2 = v0 + 1;
	if (!_vm->isFlag(kGFTwigTaken))
		_sequenceIdArr[v2++] = 0x31C;
	if (!_vm->isFlag(kGFPlatypusTalkingToAssistant))
		_sequenceIdArr[v2++] = 0x31D;
	_sequenceIdArr[v2] = 0x313;
	_sequenceIdArr[v2 + 1] = 0x315;
	_resourceIdArr[0] = 0x2B8;
	_resourceIdArr[1] = 0x20C;
	_resourceIdArr[2] = 0x2B8;
	_resourceIdArr[3] = 0x20B;
	_resourceIdArr[4] = 0x20B;
	_sequenceCountArr[0] = v4;
	_sequenceCountArr[1] = 1;
	_sequenceCountArr[2] = v2 - v3;
	_sequenceCountArr[3] = 1;
	_sequenceCountArr[4] = 1;
	_canSkip[0] = false;
	_canSkip[1] = false;
	_canSkip[2] = false;
	_canSkip[3] = false;
	_canSkip[4] = false;
	_itemsCount = 5;

	return -1;
}

} // End of namespace Gnap
