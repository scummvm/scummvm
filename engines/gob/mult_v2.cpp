/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/mult.h"
#include "gob/game.h"
#include "gob/scenery.h"
#include "gob/global.h"
#include "gob/inter.h"

namespace Gob {

Mult_v2::Mult_v2(GobEngine *vm) : Mult_v1(vm) {
}

void Mult_v2::loadMult(int16 resId) {
	int16 i, j;
	int8 index;
	char staticCount;
	char animCount;
	char *extData;
	bool hbstaticCount;
	int16 palIndex;
	int16 size;
	
	index = (resId & 0x8000) ? *_vm->_global->_inter_execPtr++ : 0;

	_multData2 = new Mult_Data;
	_multDatas[index] = _multData2;

	for (i = 0; i < 4; i++)
		_multData2->palAnimIndices[i] = i;

	_multData2->sndSlotsCount = 0;
	_multData2->frameStart = 0;

	extData = _vm->_game->loadExtData(resId, 0, 0);
	_dataPtr = extData;

	staticCount = _dataPtr[0];
	animCount = _dataPtr[1];
	_dataPtr += 2;
	staticCount++;
	animCount++;

	hbstaticCount = (staticCount & 0x80) != 0;
	staticCount &= 0x7F;

	debugC(7, DEBUG_GRAPHICS, "statics: %u, anims: %u, hb: %u", staticCount, animCount, hbstaticCount);
	for (i = 0; i < staticCount; i++, _dataPtr += 14) {
		_multData2->staticIndices[i] = _vm->_scenery->loadStatic(1);

		if (_multData2->staticIndices[i] >= 100) {
			_multData2->staticIndices[i] -= 100;
			_multData2->staticLoaded[i] = 1;
		} else {
			_multData2->staticLoaded[i] = 0;
		}
	}

	for (i = 0; i < animCount; i++, _dataPtr += 14) {
		_multData2->animIndices[i] = _vm->_scenery->loadAnim(1);

		if (_multData2->animIndices[i] >= 100) {
			_multData2->animIndices[i] -= 100;
			_multData2->animLoaded[i] = 1;
		} else {
			_multData2->animLoaded[i] = 0;
		}
	}

	_multData2->frameRate = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;

	_multData2->staticKeysCount = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;

	_multData2->staticKeys = new Mult_StaticKey[_multData2->staticKeysCount];
	for (i = 0; i < _multData2->staticKeysCount; i++, _dataPtr += 4) {
		_multData2->staticKeys[i].frame = (int16)READ_LE_UINT16(_dataPtr);
		_multData2->staticKeys[i].layer = (int16)READ_LE_UINT16(_dataPtr + 2);
	}

	for (i = 0; i < 4; i++) {
		_multData2->somepointer05size[i] = 0;
		_multData2->somepointer05[i] = 0;
		_multData2->somepointer05indices[i] = -1;

		for (j = 0; j < 4; j++) {
			_multData2->field_15F[i][j] = 0;
			_multData2->field_17F[i][j] = 0;
		}

		_multData2->animKeysIndices1[i] = -1;
		_multData2->animKeysCount[i] = READ_LE_UINT16(_dataPtr);
		_dataPtr += 2;

		_multData2->animKeys[i] = new Mult_AnimKey[_multData2->animKeysCount[i]];
		for (j = 0; j < _multData2->animKeysCount[i]; j++, _dataPtr += 10) {
			_multData2->animKeys[i][j].frame = (int16)READ_LE_UINT16(_dataPtr);
			_multData2->animKeys[i][j].layer = (int16)READ_LE_UINT16(_dataPtr + 2);
			_multData2->animKeys[i][j].posX = (int16)READ_LE_UINT16(_dataPtr + 4);
			_multData2->animKeys[i][j].posY = (int16)READ_LE_UINT16(_dataPtr + 6);
			_multData2->animKeys[i][j].order = (int16)READ_LE_UINT16(_dataPtr + 8);
		}
	}

	for (palIndex = 0; palIndex < 5; palIndex++) {
		for (i = 0; i < 16; i++) {
			_multData2->fadePal[palIndex][i].red = _dataPtr[0];
			_multData2->fadePal[palIndex][i].green = _dataPtr[1];
			_multData2->fadePal[palIndex][i].blue = _dataPtr[2];
			_dataPtr += 3;
		}
	}

	_multData2->palFadeKeysCount = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;

	_multData2->palFadeKeys = new Mult_PalFadeKey[_multData2->palFadeKeysCount];

	for (i = 0; i < _multData2->palFadeKeysCount; i++, _dataPtr += 7) {
		_multData2->palFadeKeys[i].frame = (int16)READ_LE_UINT16(_dataPtr);
		_multData2->palFadeKeys[i].fade = (int16)READ_LE_UINT16(_dataPtr + 2);
		_multData2->palFadeKeys[i].palIndex = (int16)READ_LE_UINT16(_dataPtr + 4);
		_multData2->palFadeKeys[i].flag = *(_dataPtr + 6);
	}

	_multData2->palKeysCount = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;

	_multData2->palKeys = new Mult_PalKey[_multData2->palKeysCount];

	for (i = 0; i < _multData2->palKeysCount; i++, _dataPtr += 80) {
		_multData2->palKeys[i].frame = (int16)READ_LE_UINT16(_dataPtr);
		_multData2->palKeys[i].cmd = (int16)READ_LE_UINT16(_dataPtr + 2);
		_multData2->palKeys[i].rates[0] = (int16)READ_LE_UINT16(_dataPtr + 4);
		_multData2->palKeys[i].rates[1] = (int16)READ_LE_UINT16(_dataPtr + 6);
		_multData2->palKeys[i].rates[2] = (int16)READ_LE_UINT16(_dataPtr + 8);
		_multData2->palKeys[i].rates[3] = (int16)READ_LE_UINT16(_dataPtr + 10);
		_multData2->palKeys[i].unknown0 = (int16)READ_LE_UINT16(_dataPtr + 12);
		_multData2->palKeys[i].unknown1 = (int16)READ_LE_UINT16(_dataPtr + 14);
		memcpy(_multData2->palKeys[i].subst, _dataPtr + 16, 64);
	}

	_multData2->textKeysCount = READ_LE_UINT16(_dataPtr);

	_multData2->textKeysCount = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;
	_multData2->textKeys = new Mult_TextKey[_multData2->textKeysCount];

	for (i = 0; i < _multData2->textKeysCount; i++, _dataPtr += 4) {
		_multData2->textKeys[i].frame = (int16)READ_LE_UINT16(_dataPtr);
		_multData2->textKeys[i].cmd = (int16)READ_LE_UINT16(_dataPtr + 2);
		if (!hbstaticCount)
			_dataPtr += 24;
	}

	_multData2->sndKeysCount = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;

	_multData2->sndKeys = new Mult_SndKey[_multData2->sndKeysCount];

	warning("SoundKeyCount: %d", _multData2->sndKeysCount);

	// TODO: There's still something wrong here, preventing GOB2 floppy
	//       to start correctly
	for (i = 0; i < _multData2->sndKeysCount; i++) {
		warning("-> %d", i);
		_multData2->sndKeys[i].frame = (int16)READ_LE_UINT16(_dataPtr);
		_multData2->sndKeys[i].cmd = (int16)READ_LE_UINT16(_dataPtr + 2);
		_multData2->sndKeys[i].freq = (int16)READ_LE_UINT16(_dataPtr + 4);
		_multData2->sndKeys[i].channel = (int16)READ_LE_UINT16(_dataPtr + 6);
		_multData2->sndKeys[i].repCount = (int16)READ_LE_UINT16(_dataPtr + 8);
		_multData2->sndKeys[i].resId = (int16)READ_LE_UINT16(_dataPtr + 10);
		_multData2->sndKeys[i].soundIndex = (int16)READ_LE_UINT16(_dataPtr + 12);

		_multData2->sndKeys[i].soundIndex = -1;
		_multData2->sndKeys[i].resId = -1;
		_dataPtr += 12;
		if (!hbstaticCount)
			_dataPtr += 24;

		switch (_multData2->sndKeys[i].cmd) {
		case 1:
		case 4:
			_multData2->sndKeys[i].resId = READ_LE_UINT16(_vm->_global->_inter_execPtr);

			for (j = 0; j < i; j++) {
				if (_multData2->sndKeys[i].resId ==
				    _multData2->sndKeys[j].resId) {
					_multData2->sndKeys[i].soundIndex =
					    _multData2->sndKeys[j].soundIndex;
					_vm->_global->_inter_execPtr += 2;
					break;
				}
			}
			if (i == j) {
				warning("GOB2 Stub! Mult_Data.sndSlot");
				warning("GOB2 Stub! Game::interLoadSound() differs!");
				_multData2->sndSlot[_multData2->sndSlotsCount] = _vm->_inter->loadSound(1);
				_vm->_inter->loadSound(1);
				// _multData2->sndKeys[i].soundIndex = _multData2->sndSlot[_multData2->sndSlotsCount] & 0x7FFF;
				_multData2->sndSlotsCount++;
			}

			break;
		case 3:
			_vm->_global->_inter_execPtr += 4;
			break;
		}
	}
	
	_multData2->somepointer09 = 0;
	_multData2->somepointer10 = 0;

	if (hbstaticCount) {
		warning("GOB2 Stub! Mult_Data.somepointer09, Mult_Data.somepointer10");
		size = _vm->_inter->load16();
		_multData2->execPtr = _vm->_global->_inter_execPtr;
		_vm->_global->_inter_execPtr += size * 2;
		if (_vm->_game->_totFileData[0x29] >= 51) {
			size = (int16)READ_LE_UINT16(_dataPtr);
			_multData2->somepointer10 = new char[size * 20];
			memcpy(_multData2->somepointer09 /*WTF???*/, _dataPtr+2, size * 20);
			_dataPtr += size * 20 + 2;
			size = _vm->_inter->load16();
			if (size > 0) {
				_multData2->somepointer09 = new char[size * 14];
				memcpy(_multData2->somepointer09, _vm->_global->_inter_execPtr, size * 14);
				_vm->_global->_inter_execPtr += size * 14;
				_dataPtr += 2;
				for (i = 0; i < 4; i++) {
					_multData2->somepointer05size[i] = (int16)READ_LE_UINT16(_dataPtr);
					_dataPtr += 2;
					_multData2->somepointer05[i] = new char[_multData2->somepointer05size[i] * 16];
					for (j = 0; j < _multData2->somepointer05size[i]; j++) {
						memcpy(_multData2->somepointer05[i]+j*16, _dataPtr, 16);
						_dataPtr += 16;
					}
				}
			}
		}
	}

	delete[] extData;
}

void Mult_v2::setMultData(uint16 multindex) {
	if (multindex > 7)
		error("Multindex out of range");

	debugC(4, DEBUG_GAMEFLOW, "Switching to mult %d", multindex);
	_multData2 = _multDatas[multindex];
}

} // End of namespace Gob
