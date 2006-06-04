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
#include "gob/scenery.h"

namespace Gob {

Map_v2::Map_v2(GobEngine *vm) : Map_v1(vm) {
}

void Map_v2::loadMapObjects(char *avjFile) {
	int i;
	int j;
	int k;
	uint8 wayPointsCount;
	int16 var;
	int16 id;
	int16 mapHeight;
	int16 mapWidth;
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
	int8 statesMask[102];
	Goblin::Gob2_State *statesPtr;

	var = _vm->_parse->parseVarIndex();
	variables = _vm->_global->_inter_variables + var;

	id = _vm->_inter->load16();

	if (id == -1) {
		_passMap = (int8 *)(_vm->_global->_inter_variables + var);
		return;
	}

	extData = _vm->_game->loadExtData(id, 0, 0);
	dataPtr = extData;

	if (*dataPtr++ == 3) {
		_vm->_map->_screenWidth = 640;
		_vm->_map->_passWidth = 65;
	} else {
		_vm->_map->_screenWidth = 320;
		_vm->_map->_passWidth = 40;
	}
	_wayPointsCount = *dataPtr++;
	_vm->_map->_tilesWidth = READ_LE_UINT16(dataPtr);
	dataPtr += 2;
	_vm->_map->_tilesHeight = READ_LE_UINT16(dataPtr);
	dataPtr += 2;

	_vm->_map->_bigTiles = !(_vm->_map->_tilesHeight & 0xFF00);
	_vm->_map->_tilesHeight &= 0xFF;

	_mapWidth = _vm->_map->_screenWidth / _vm->_map->_tilesWidth;
	_mapHeight = 200 / _vm->_map->_tilesHeight;

	dataPtrBak = dataPtr;
	dataPtr += _mapWidth * _mapHeight;

	if (*extData == 1)
		wayPointsCount = _wayPointsCount = 40;
	else
		wayPointsCount = _wayPointsCount == 0 ? 1 : _wayPointsCount;

	_wayPoints = new Point[wayPointsCount];
	for (i = 0; i < wayPointsCount; i++) {
		_wayPoints[i].x = -1;
		_wayPoints[i].y = -1;
		_wayPoints[i].field_2 = -1;
	}
	for (i = 0; i < _wayPointsCount; i++) {
		_wayPoints[i].x = *dataPtr++;
		_wayPoints[i].y = *dataPtr++;
		_wayPoints[i].field_2 = *dataPtr++;
	}

	// In the original asm, this writes byte-wise into the variables-array
	if (variables != _vm->_global->_inter_variables) {
		_passMap = (int8 *) variables;
		mapHeight = 200 / _vm->_map->_tilesHeight;
		mapWidth = _vm->_map->_screenWidth / _vm->_map->_tilesWidth;
		for (i = 0; i < mapHeight; i++) {
			offData = (mapWidth * i);
			for (j = 0; j < mapWidth; j++) {
				setPass(j, i, *(dataPtrBak + offData + j), _vm->_map->_passWidth);
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
					_vm->_mult->_objects[i].goblinStates[state][k].animation = *((byte *) dataPtr) << 8;
					dataPtr += 2;
					_vm->_mult->_objects[i].goblinStates[state][k].animation += *((byte *) dataPtr);
					dataPtr += 2;
					_vm->_mult->_objects[i].goblinStates[state][k].layer = *((byte *) dataPtr) << 8;
					dataPtr += 2;
					_vm->_mult->_objects[i].goblinStates[state][k].layer += *((byte *) dataPtr);
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

void Map_v2::findNearestToGob(int16 index) {
	Mult::Mult_Object *obj = &_vm->_mult->_objects[index];
	int16 wayPoint = findNearestWayPoint(obj->goblinX, obj->goblinY);

	if (wayPoint != -1)
		obj->nearestWayPoint = wayPoint;
}

void Map_v2::findNearestToDest(int16 index) {
	Mult::Mult_Object *obj = &_vm->_mult->_objects[index];
	int16 wayPoint = findNearestWayPoint(obj->destX, obj->destY);

	if (wayPoint != -1)
		obj->nearestDest = wayPoint;
}

void Map_v2::optimizePoints(int16 index, int16 x, int16 y) {
	Mult::Mult_Object *obj;
	int i;
	
	int16 var_2;

	obj = &_vm->_mult->_objects[index];

	if (obj->nearestWayPoint < obj->nearestDest) {
		var_2 = obj->nearestWayPoint;
		for (i = obj->nearestWayPoint; i <= obj->nearestDest; i++) {
			if (checkDirectPath(index, x, y, _wayPoints[i].x, _wayPoints[i].y) == 1)
				obj->nearestWayPoint = i;
		}
	} else {
		for (i = obj->nearestWayPoint; i >= obj->nearestDest && _wayPoints[i].field_2 != 1; i--) {
			if (checkDirectPath(index, x, y, _wayPoints[i].x, _wayPoints[i].y) == 1)
				obj->nearestWayPoint = i;
		}
	}
}

} // End of namespace Gob
