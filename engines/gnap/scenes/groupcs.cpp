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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gnap/gnap.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"
#include "gnap/scenes/groupcs.h"


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

/*****************************************************************************/

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

/*****************************************************************************/

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

/*****************************************************************************/

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
