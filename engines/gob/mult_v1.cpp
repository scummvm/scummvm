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

#include "gob/gob.h"
#include "gob/mult.h"
#include "gob/game.h"
#include "gob/scenery.h"
#include "gob/global.h"

namespace Gob {

Mult_v1::Mult_v1(GobEngine *vm) : Mult(vm) {
}

void Mult_v1::loadMult(int16 resId) {
	char animCount;
	char staticCount;
	int16 palIndex;
	int16 i, j;

	_sndSlotsCount = 0;
	_frameStart = 0;
	_multData = _vm->_game->loadExtData(resId, 0, 0);
	_dataPtr = _multData;

	staticCount = _dataPtr[0];
	animCount = _dataPtr[1];
	_dataPtr += 2;
	staticCount++;
	animCount++;

	for (i = 0; i < staticCount; i++, _dataPtr += 14) {
		_staticIndices[i] = _vm->_scenery->loadStatic(1);

		if (_staticIndices[i] >= 100) {
			_staticIndices[i] -= 100;
			_staticLoaded[i] = 1;
		} else {
			_staticLoaded[i] = 0;
		}
	}

	for (i = 0; i < animCount; i++, _dataPtr += 14) {
		_animIndices[i] = _vm->_scenery->loadAnim(1);

		if (_animIndices[i] >= 100) {
			_animIndices[i] -= 100;
			_animLoaded[i] = 1;
		} else {
			_animLoaded[i] = 0;
		}
	}

	_frameRate = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;

	_staticKeysCount = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;

	_staticKeys = new Mult_StaticKey[_staticKeysCount];
	for (i = 0; i < _staticKeysCount; i++, _dataPtr += 4) {
		_staticKeys[i].frame = (int16)READ_LE_UINT16(_dataPtr);
		_staticKeys[i].layer = (int16)READ_LE_UINT16(_dataPtr + 2);
	}

	for (j = 0; j < 4; j++) {
		_animKeysCount[j] = READ_LE_UINT16(_dataPtr);
		_dataPtr += 2;

		_animKeys[j] = new Mult_AnimKey[_animKeysCount[j]];
		for (i = 0; i < _animKeysCount[j]; i++, _dataPtr += 10) {
			_animKeys[j][i].frame = (int16)READ_LE_UINT16(_dataPtr);
			_animKeys[j][i].layer = (int16)READ_LE_UINT16(_dataPtr + 2);
			_animKeys[j][i].posX = (int16)READ_LE_UINT16(_dataPtr + 4);
			_animKeys[j][i].posY = (int16)READ_LE_UINT16(_dataPtr + 6);
			_animKeys[j][i].order = (int16)READ_LE_UINT16(_dataPtr + 8);
		}
	}

	for (palIndex = 0; palIndex < 5; palIndex++) {
		for (i = 0; i < 16; i++) {
			_fadePal[palIndex][i].red = _dataPtr[0];
			_fadePal[palIndex][i].green = _dataPtr[1];
			_fadePal[palIndex][i].blue = _dataPtr[2];
			_dataPtr += 3;
		}
	}

	_palFadeKeysCount = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;
	_palFadeKeys = new Mult_PalFadeKey[_palFadeKeysCount];

	for (i = 0; i < _palFadeKeysCount; i++, _dataPtr += 7) {
		_palFadeKeys[i].frame = (int16)READ_LE_UINT16(_dataPtr);
		_palFadeKeys[i].fade = (int16)READ_LE_UINT16(_dataPtr + 2);
		_palFadeKeys[i].palIndex = (int16)READ_LE_UINT16(_dataPtr + 4);
		_palFadeKeys[i].flag = *(_dataPtr + 6);
	}

	_palKeysCount = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;

	_palKeys = new Mult_PalKey[_palKeysCount];
	for (i = 0; i < _palKeysCount; i++, _dataPtr += 80) {
		_palKeys[i].frame = (int16)READ_LE_UINT16(_dataPtr);
		_palKeys[i].cmd = (int16)READ_LE_UINT16(_dataPtr + 2);
		_palKeys[i].rates[0] = (int16)READ_LE_UINT16(_dataPtr + 4);
		_palKeys[i].rates[1] = (int16)READ_LE_UINT16(_dataPtr + 6);
		_palKeys[i].rates[2] = (int16)READ_LE_UINT16(_dataPtr + 8);
		_palKeys[i].rates[3] = (int16)READ_LE_UINT16(_dataPtr + 10);
		_palKeys[i].unknown0 = (int16)READ_LE_UINT16(_dataPtr + 12);
		_palKeys[i].unknown1 = (int16)READ_LE_UINT16(_dataPtr + 14);
		memcpy(_palKeys[i].subst, _dataPtr + 16, 64);
	}

	_textKeysCount = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;
	_textKeys = new Mult_TextKey[_textKeysCount];

	for (i = 0; i < _textKeysCount; i++, _dataPtr += 28) {
		_textKeys[i].frame = (int16)READ_LE_UINT16(_dataPtr);
		_textKeys[i].cmd = (int16)READ_LE_UINT16(_dataPtr + 2);
		for (int k = 0; k < 9; ++k)
			_textKeys[i].unknown0[k] = (int16)READ_LE_UINT16(_dataPtr + 4 + (k * 2));
		_textKeys[i].index = (int16)READ_LE_UINT16(_dataPtr + 22);
		_textKeys[i].unknown1[0] = (int16)READ_LE_UINT16(_dataPtr + 24);
		_textKeys[i].unknown1[1] = (int16)READ_LE_UINT16(_dataPtr + 26);
	}

	_sndKeysCount = READ_LE_UINT16(_dataPtr);
	_dataPtr += 2;

	_sndKeys = new Mult_SndKey[_sndKeysCount];
	for (i = 0; i < _sndKeysCount; i++) {
		_sndKeys[i].frame = (int16)READ_LE_UINT16(_dataPtr);
		_sndKeys[i].cmd = (int16)READ_LE_UINT16(_dataPtr + 2);
		_sndKeys[i].freq = (int16)READ_LE_UINT16(_dataPtr + 4);
		_sndKeys[i].channel = (int16)READ_LE_UINT16(_dataPtr + 6);
		_sndKeys[i].repCount = (int16)READ_LE_UINT16(_dataPtr + 8);
		_sndKeys[i].resId = (int16)READ_LE_UINT16(_dataPtr + 10);
		_sndKeys[i].soundIndex = (int16)READ_LE_UINT16(_dataPtr + 12);

		_sndKeys[i].soundIndex = -1;
		_sndKeys[i].resId = -1;
		_dataPtr += 36;
		switch (_sndKeys[i].cmd) {
		case 1:
		case 4:
			_sndKeys[i].resId = READ_LE_UINT16(_vm->_global->_inter_execPtr);

			for (j = 0; j < i; j++) {
				if (_sndKeys[i].resId ==
				    _sndKeys[j].resId) {
					_sndKeys[i].soundIndex =
					    _sndKeys[j].soundIndex;
					_vm->_global->_inter_execPtr += 2;
					break;
				}
			}
			if (i == j) {
				_vm->_game->interLoadSound(19 - _sndSlotsCount);
				_sndKeys[i].soundIndex =
				    19 - _sndSlotsCount;
				_sndSlotsCount++;
			}
			break;

		case 3:
			_vm->_global->_inter_execPtr += 6;
			break;

		case 5:
			_vm->_global->_inter_execPtr += _sndKeys[i].freq * 2;
			break;
		}
	}
}

} // End of namespace Gob
