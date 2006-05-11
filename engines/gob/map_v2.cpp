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
#include "gob/map.h"
#include "gob/dataio.h"
#include "gob/goblin.h"
#include "gob/sound.h"
#include "gob/inter.h"
#include "gob/game.h"
#include "gob/parse.h"
#include "gob/mult.h"

namespace Gob {

Map_v2::Map_v2(GobEngine *vm) : Map_v1(vm) {
}

void Map_v2::loadMapObjects(char *avjFile) {
	int i;
	int j;
	int k;
	int16 var;
	int16 id;
	int16 numChunks;
	int16 chunkLength;
	int16 offVar;
	int16 offData;
	int16 tmp;
	int16 numData;
	int16 statesCount;
	int16 state;
	char *variables;
	char *extData;
	char *dataPtr;
	char *dataPtrBak;
	char *dataPtrBak2;
	char statesMask[102];
	Goblin::Gob2_State *statesPtr;

	uint8 var_9;
	uint8 byte_2F2AA;
	byte *off_2F2AB;

	var = _vm->_parse->parseVarIndex();
	variables = _vm->_global->_inter_variables + var;

	id = _vm->_inter->load16();

	if (id == -1) {
		_vm->_goblin->_dword_2F2A4 = _vm->_global->_inter_variables + var;
		return;
	}

	extData = _vm->_game->loadExtData(id, 0, 0);
	dataPtr = extData;

	if (*dataPtr++ == 3) {
		_vm->_mult->_word_2F22A = 640;
		_vm->_mult->_word_2CC84 = 65;
	} else {
		_vm->_mult->_word_2F22A = 320;
		_vm->_mult->_word_2CC84 = 40;
	}
	byte_2F2AA = *dataPtr++;
	_vm->_mult->_word_2F2B1 = READ_LE_UINT16(dataPtr);
	dataPtr += 2;
	_vm->_mult->_word_2F2AF = READ_LE_UINT16(dataPtr);
	dataPtr += 2;

	_vm->_mult->_word_2CC86 = _vm->_mult->_word_2F2AF & 0xFF00 ? 0 : 1;
	_vm->_mult->_word_2F2AF &= 0xFF;

	dataPtrBak = dataPtr;
	dataPtr += (_vm->_mult->_word_2F22A / _vm->_mult->_word_2F2B1) * (200 / _vm->_mult->_word_2F2AF);

	if (*extData == 1) {
		byte_2F2AA = 40;
		var_9 = 40;
	} else {
		if (byte_2F2AA == 0) {
			var_9 = 1;
		} else {
			var_9 = byte_2F2AA;
		}
	}

	off_2F2AB = new byte[3 * var_9];
	memset(off_2F2AB, -1, 3 * var_9);
	memcpy(off_2F2AB, dataPtr, 3 * byte_2F2AA);
	dataPtr += 3 * byte_2F2AA;

	// In the original asm, this writes byte-wise into the variables-array
	if (variables != _vm->_global->_inter_variables) {
		_vm->_goblin->_dword_2F2A4 = variables;
		numChunks = 200 / _vm->_mult->_word_2F2AF;
		chunkLength = _vm->_mult->_word_2F22A / _vm->_mult->_word_2F2B1;
		for (i = 0; i < numChunks; i++) {
			offVar = _vm->_mult->_word_2CC84 * i;
			offData = (chunkLength * i);
			for (j = 0; j < chunkLength; j++) {
				_vm->_util->writeVariableByte(_vm->_goblin->_dword_2F2A4 + offVar + j,
						*(dataPtrBak + offData + j));
			}
		}
	}

	tmp = READ_LE_UINT16(dataPtr);
	dataPtr += tmp * 14 + 2;
	tmp = READ_LE_UINT16(dataPtr);
	dataPtr += tmp * 14 + 2;
	dataPtr += 28;
	tmp = READ_LE_UINT16(dataPtr);
	dataPtr += tmp * 14 + 2;

	_vm->_goblin->_gobsCount = tmp;
	for (i = 0; i < _vm->_goblin->_gobsCount; i++) {
		memset(statesMask, -1, 101);
		_vm->_mult->_objects[i].goblinStates = new Goblin::Gob2_State*[101];
		memset(_vm->_mult->_objects[i].goblinStates, 0, 101 * sizeof(Goblin::Gob2_State *));
		memcpy(statesMask, dataPtr, 100);
		dataPtr += 100;
		dataPtrBak2 = dataPtr;
		statesCount = 0;
		for (j = 0; j < 100; j++) {
			if (statesMask[j] != -1) {
				statesCount++;
				dataPtr += 4;
				numData = *dataPtr++;
				statesCount += numData;
				dataPtr += numData * 9;
			}
		}
		statesPtr = new Goblin::Gob2_State[statesCount];
		_vm->_mult->_objects[i].goblinStates[0] = statesPtr;
		dataPtr = dataPtrBak2;
		for (j = 0; j < 100; j++) {
			state = statesMask[j];
			if (state != -1) {
				_vm->_mult->_objects[i].goblinStates[state] = statesPtr++;
				_vm->_mult->_objects[i].goblinStates[state][0].animation = READ_LE_UINT16(dataPtr);
				dataPtr += 2;
				_vm->_mult->_objects[i].goblinStates[state][0].layer = READ_LE_UINT16(dataPtr);
				dataPtr += 2;
				numData = *dataPtr++;
				_vm->_mult->_objects[i].goblinStates[state][0].field_4 = numData;
				for (k = 0; k < numData; k++) {
					dataPtr++;
					_vm->_mult->_objects[i].goblinStates[state][k].animation = *dataPtr << 8;
					dataPtr += 2;
					_vm->_mult->_objects[i].goblinStates[state][k].animation += *dataPtr;
					dataPtr += 2;
					_vm->_mult->_objects[i].goblinStates[state][k].layer = *dataPtr << 8;
					dataPtr += 2;
					_vm->_mult->_objects[i].goblinStates[state][k].layer += *dataPtr;
					_vm->_mult->_objects[i].goblinStates[state][k].field_4 = READ_LE_UINT16(dataPtr);
					dataPtr += 2;
					statesPtr++;
				}
			}
		}
	}

	_vm->_goblin->_soundSlotsCount = _vm->_inter->load16();
	for (i = 0; i < _vm->_goblin->_soundSlotsCount; i++)
		_vm->_goblin->_soundSlots[i] = _vm->_inter->loadSound(1);
}

} // End of namespace Gob
