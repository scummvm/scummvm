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
#include "common/stream.h"

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

Map_v2::~Map_v2() {
	_passMap = 0;
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
	int16 tmp;
	int16 numData;
	int16 statesCount;
	int16 state;
	char *variables;
	char *extData;
	uint32 dataPos1;
	uint32 dataPos2;
	int8 statesMask[102];
	Mult::Mult_GobState *statesPtr;

	var = _vm->_parse->parseVarIndex();
	variables = _vm->_global->_inter_variables + var;

	id = _vm->_inter->load16();

	if (id == -1) {
		_passMap = (int8 *)(_vm->_global->_inter_variables + var);
		return;
	}

	extData = _vm->_game->loadExtData(id, 0, 0);
	Common::MemoryReadStream mapData((byte *) extData, 4294967295U);

	if (mapData.readByte() == 3) {
		_screenWidth = 640;
		_passWidth = 65;
	} else {
		_screenWidth = 320;
		_passWidth = 40;
	}
	_wayPointsCount = mapData.readByte();
	_tilesWidth = mapData.readSint16LE();
	_tilesHeight = mapData.readSint16LE();

	_bigTiles = !(_tilesHeight & 0xFF00);
	_tilesHeight &= 0xFF;

	_mapWidth = _screenWidth / _tilesWidth;
	_mapHeight = 200 / _tilesHeight;

	dataPos2 = mapData.pos();
	mapData.seek(_mapWidth * _mapHeight, SEEK_CUR);

	if (*extData == 1)
		wayPointsCount = _wayPointsCount = 40;
	else
		wayPointsCount = _wayPointsCount == 0 ? 1 : _wayPointsCount;

	_wayPoints = new Point[wayPointsCount];
	for (i = 0; i < _wayPointsCount; i++) {
		_wayPoints[i].x = mapData.readSByte();
		_wayPoints[i].y = mapData.readSByte();
		_wayPoints[i].field_2 = mapData.readSByte();
	}

	// In the original asm, this writes byte-wise into the variables-array
	dataPos1 = mapData.pos();
	if (variables != _vm->_global->_inter_variables) {
		_passMap = (int8 *) variables;
		mapHeight = 200 / _tilesHeight;
		mapWidth = _screenWidth / _tilesWidth;
		for (i = 0; i < mapHeight; i++) {
			mapData.seek(dataPos2 + (mapWidth * i));
			for (j = 0; j < mapWidth; j++) {
				setPass(j, i, mapData.readSByte());
			}
		}
	}
	mapData.seek(dataPos1);

	tmp = mapData.readSint16LE();
	mapData.seek(tmp * 14, SEEK_CUR);
	tmp = mapData.readSint16LE();
	mapData.seek(tmp * 14 + 28, SEEK_CUR);
	tmp = mapData.readSint16LE();
	mapData.seek(tmp * 14, SEEK_CUR);

	_vm->_goblin->_gobsCount = tmp;
	for (i = 0; i < _vm->_goblin->_gobsCount; i++) {
		memset(statesMask, -1, 101);
		_vm->_mult->_objects[i].goblinStates = new Mult::Mult_GobState*[101];
		memset(_vm->_mult->_objects[i].goblinStates, 0, 101 * sizeof(Mult::Mult_GobState *));
		mapData.read(statesMask, 100);
		dataPos1 = mapData.pos();
		statesCount = 0;
		for (j = 0; j < 100; j++) {
			if (statesMask[j] != -1) {
				statesCount++;
				mapData.seek(4, SEEK_CUR);
				numData = mapData.readByte();
				statesCount += numData;
				mapData.seek(numData * 9, SEEK_CUR);
			}
		}
		statesPtr = new Mult::Mult_GobState[statesCount];
		_vm->_mult->_objects[i].goblinStates[0] = statesPtr;
		mapData.seek(dataPos1);
		for (j = 0; j < 100; j++) {
			state = statesMask[j];
			if (state != -1) {
				_vm->_mult->_objects[i].goblinStates[state] = statesPtr++;
				_vm->_mult->_objects[i].goblinStates[state][0].animation = mapData.readSint16LE();
				_vm->_mult->_objects[i].goblinStates[state][0].layer = mapData.readSint16LE();
				numData = mapData.readByte();
				_vm->_mult->_objects[i].goblinStates[state][0].dataCount = numData;
				for (k = 1; k <= numData; k++) {
					mapData.seek(1, SEEK_CUR);
					_vm->_mult->_objects[i].goblinStates[state][k].sndItem = mapData.readSByte();
					mapData.seek(1, SEEK_CUR);
					_vm->_mult->_objects[i].goblinStates[state][k].sndFrame = mapData.readByte();
					mapData.seek(1, SEEK_CUR);
					_vm->_mult->_objects[i].goblinStates[state][k].freq = mapData.readSint16LE();
					_vm->_mult->_objects[i].goblinStates[state][k].repCount = mapData.readSByte();
					_vm->_mult->_objects[i].goblinStates[state][k].speaker = mapData.readByte();
					statesPtr++;
				}
			}
		}
	}

	_vm->_goblin->_soundSlotsCount = _vm->_inter->load16();
	for (i = 0; i < _vm->_goblin->_soundSlotsCount; i++)
		_vm->_goblin->_soundSlots[i] = _vm->_inter->loadSound(1);
}

void Map_v2::findNearestToGob(Mult::Mult_Object *obj) {
	int16 wayPoint = findNearestWayPoint(obj->goblinX, obj->goblinY);

	if (wayPoint != -1)
		obj->nearestWayPoint = wayPoint;
}

void Map_v2::findNearestToDest(Mult::Mult_Object *obj) {
	int16 wayPoint = findNearestWayPoint(obj->destX, obj->destY);

	if (wayPoint != -1)
		obj->nearestDest = wayPoint;
}

void Map_v2::optimizePoints(Mult::Mult_Object *obj, int16 x, int16 y) {
	int i;
	int16 var_2;

	if (obj->nearestWayPoint < obj->nearestDest) {
		var_2 = obj->nearestWayPoint;
		for (i = obj->nearestWayPoint; i <= obj->nearestDest; i++) {
			if (checkDirectPath(obj, x, y, _wayPoints[i].x, _wayPoints[i].y) == 1)
				obj->nearestWayPoint = i;
		}
	} else {
		for (i = obj->nearestWayPoint; i >= obj->nearestDest && _wayPoints[i].field_2 != 1; i--) {
			if (checkDirectPath(obj, x, y, _wayPoints[i].x, _wayPoints[i].y) == 1)
				obj->nearestWayPoint = i;
		}
	}
}

} // End of namespace Gob
