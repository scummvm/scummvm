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
#include "gob/scenery.h"
#include "gob/mult.h"

namespace Gob {

Map_v1::Map_v1(GobEngine *vm) : Map(vm) {
	int i;
	int j;

	_mapWidth = 26;
	_mapHeight = 28;

	_passMap = new int8[_mapHeight * _mapWidth];
	_itemsMap = new int16*[_mapHeight];
	for (i = 0; i < _mapHeight; i++) {
		_itemsMap[i] = new int16[_mapWidth];
		for (j = 0; j < _mapWidth; j++) {
			setPass(j, i, 0);
			_itemsMap[i][j] = 0;
		}
	}

	_wayPointsCount = 40;
	_wayPoints = new Point[40];
	for (i = 0; i < 40; i++) {
		_wayPoints[i].x = 0;
		_wayPoints[i].y = 0;
		_wayPoints[i].field_2 = 0;
	}
}

Map_v1::~Map_v1() {
	delete[] _passMap;
}

void Map_v1::loadMapObjects(char *avjFile) {
	int16 i;
	char avoName[128];
	int16 handle;
	char item;
	int16 soundCount;
	int16 tmp;
	char *savedPtr;
	char *savedPtr2;
	char *savedPtr3;
	int16 state;
	int16 col;
	int32 flag;
	Goblin::Gob_State *pState;
	char buf[128];
	char sndNames[20][14];
	char *dataBuf;
	int16 x;
	int16 y;
	int16 count2;
	int16 count3;

	strcpy(avoName, _sourceFile);
	strcat(avoName, ".avo");

	handle = _vm->_dataio->openData(avoName);
	if (handle >= 0) {
		_loadFromAvo = 1;
		_vm->_dataio->closeData(handle);
		_avoDataPtr = _vm->_dataio->getData(avoName);
		dataBuf = _avoDataPtr;
		loadDataFromAvo((char *)_passMap, _mapHeight * _mapWidth);

		for (y = 0; y < _mapHeight; y++) {
			for (x = 0; x < _mapWidth; x++) {
				loadDataFromAvo(&item, 1);
				_itemsMap[y][x] = item;
			}
		}

		for (i = 0; i < 40; i++) {
			_wayPoints[i].x = loadFromAvo_LE_UINT16();
			_wayPoints[i].y = loadFromAvo_LE_UINT16();
		}
		loadDataFromAvo((char *)_itemPoses, szMap_ItemPos * 20);
	} else {
		_loadFromAvo = 0;
		_avoDataPtr = _vm->_dataio->getData(avjFile);
		dataBuf = _avoDataPtr;
	}

	_avoDataPtr += 32;
	_avoDataPtr += 76;
	_avoDataPtr += 4;
	_avoDataPtr += 20;

	for (i = 0; i < 3; i++) {
		tmp = loadFromAvo_LE_UINT16();
		_avoDataPtr += tmp * 14;
	}

	soundCount = loadFromAvo_LE_UINT16();
	savedPtr = _avoDataPtr;

	_avoDataPtr += 14 * soundCount;
	_avoDataPtr += 4;
	_avoDataPtr += 24;

	count2 = loadFromAvo_LE_UINT16();
	count3 = loadFromAvo_LE_UINT16();

	savedPtr2 = _avoDataPtr;
	_avoDataPtr += count2 * 8;

	savedPtr3 = _avoDataPtr;
	_avoDataPtr += count3 * 8;

	_vm->_goblin->_gobsCount = loadFromAvo_LE_UINT16();
	for (i = 0; i < _vm->_goblin->_gobsCount; i++) {
		_vm->_goblin->_goblins[i] = new Goblin::Gob_Object;

		_vm->_goblin->_goblins[i]->xPos = READ_LE_UINT16(savedPtr2);
		savedPtr2 += 2;

		_vm->_goblin->_goblins[i]->yPos = READ_LE_UINT16(savedPtr2);
		savedPtr2 += 2;

		_vm->_goblin->_goblins[i]->order = READ_LE_UINT16(savedPtr2);
		savedPtr2 += 2;

		_vm->_goblin->_goblins[i]->state = READ_LE_UINT16(savedPtr2);
		savedPtr2 += 2;

		if (i == 3)
			_vm->_goblin->_goblins[i]->stateMach = new Goblin::Gob_StateLine[70];
		else
			_vm->_goblin->_goblins[i]->stateMach = new Goblin::Gob_StateLine[40];

		uint32* tempstatedata = new uint32[40*6];
		for (state = 0; state < 40; ++state) {
			for (col = 0; col < 6; ++col) {
				tempstatedata[state*6+col] = READ_LE_UINT32(_avoDataPtr);
				_avoDataPtr += 4;
			}
		}
		_avoDataPtr += 160;
		_vm->_goblin->_goblins[i]->multObjIndex = *_avoDataPtr;
		_avoDataPtr += 2;

		_vm->_goblin->_goblins[i]->realStateMach = _vm->_goblin->_goblins[i]->stateMach;
		for (state = 0; state < 40; state++) {
			for (col = 0; col < 6; col++) {
				if (tempstatedata[state*6+col] == 0) {
					_vm->_goblin->_goblins[i]->stateMach[state][col] = 0;
					continue;
				}

				Goblin::Gob_State *tmpState = new Goblin::Gob_State;
				_vm->_goblin->_goblins[i]->stateMach[state][col] = tmpState;

				tmpState->animation = loadFromAvo_LE_UINT16();
				tmpState->layer = loadFromAvo_LE_UINT16();
				_avoDataPtr += 8;
				tmpState->unk0 = loadFromAvo_LE_UINT16();
				tmpState->unk1 = loadFromAvo_LE_UINT16();

				_avoDataPtr += 2;
				if (READ_LE_UINT32(_avoDataPtr) != 0) {
					_avoDataPtr += 4;
					tmpState->sndItem = loadFromAvo_LE_UINT16();
				} else {
					_avoDataPtr += 6;
					tmpState->sndItem = -1;
				}
				tmpState->freq = loadFromAvo_LE_UINT16();
				tmpState->repCount = loadFromAvo_LE_UINT16();
				tmpState->sndFrame = loadFromAvo_LE_UINT16();
			}
		}
		delete[] tempstatedata;
	}

	pState = new Goblin::Gob_State;
	_vm->_goblin->_goblins[0]->stateMach[39][0] = pState;
	pState->animation = 0;
	pState->layer = 98;
	pState->unk0 = 0;
	pState->unk1 = 0;
	pState->sndItem = -1;

	pState = new Goblin::Gob_State;
	_vm->_goblin->_goblins[1]->stateMach[39][0] = pState;
	pState->animation = 0;
	pState->layer = 99;
	pState->unk0 = 0;
	pState->unk1 = 0;
	pState->sndItem = -1;

	pState = new Goblin::Gob_State;
	_vm->_goblin->_goblins[2]->stateMach[39][0] = pState;
	pState->animation = 0;
	pState->layer = 100;
	pState->unk0 = 0;
	pState->unk1 = 0;
	pState->sndItem = -1;

	_vm->_goblin->_goblins[2]->stateMach[10][0]->sndFrame = 13;
	_vm->_goblin->_goblins[2]->stateMach[11][0]->sndFrame = 13;
	_vm->_goblin->_goblins[2]->stateMach[28][0]->sndFrame = 13;
	_vm->_goblin->_goblins[2]->stateMach[29][0]->sndFrame = 13;

	_vm->_goblin->_goblins[1]->stateMach[10][0]->sndFrame = 13;
	_vm->_goblin->_goblins[1]->stateMach[11][0]->sndFrame = 13;

	for (state = 40; state < 70; state++) {
		pState = new Goblin::Gob_State;
		_vm->_goblin->_goblins[3]->stateMach[state][0] = pState;
		_vm->_goblin->_goblins[3]->stateMach[state][1] = 0;

		pState->animation = 9;
		pState->layer = state - 40;
		pState->sndItem = -1;
		pState->sndFrame = 0;
	}

	_vm->_goblin->_objCount = loadFromAvo_LE_UINT16();
	for (i = 0; i < _vm->_goblin->_objCount; i++) {
		_vm->_goblin->_objects[i] = new Goblin::Gob_Object;

		_vm->_goblin->_objects[i]->xPos = READ_LE_UINT16(savedPtr3);
		savedPtr3 += 2;

		_vm->_goblin->_objects[i]->yPos = READ_LE_UINT16(savedPtr3);
		savedPtr3 += 2;

		_vm->_goblin->_objects[i]->order = READ_LE_UINT16(savedPtr3);
		savedPtr3 += 2;

		_vm->_goblin->_objects[i]->state = READ_LE_UINT16(savedPtr3);
		savedPtr3 += 2;

		_vm->_goblin->_objects[i]->stateMach = new Goblin::Gob_StateLine[40];

		uint32* tempstatedata = new uint32[40*6];
		for (state = 0; state < 40; ++state) {
			for (col = 0; col < 6; ++col) {
				tempstatedata[state*6+col] = READ_LE_UINT32(_avoDataPtr);
				_avoDataPtr += 4;
			}
		}
		_avoDataPtr += 160;
		_vm->_goblin->_objects[i]->multObjIndex = *_avoDataPtr;
		_avoDataPtr += 2;

		_vm->_goblin->_objects[i]->realStateMach = _vm->_goblin->_objects[i]->stateMach;
		for (state = 0; state < 40; state++) {
			for (col = 0; col < 6; col++) {
				if (tempstatedata[state*6+col] == 0) {
					_vm->_goblin->_objects[i]->stateMach[state][col] = 0;
					continue;
				}

				Goblin::Gob_State *tmpState = new Goblin::Gob_State;
				_vm->_goblin->_objects[i]->stateMach[state][col] = tmpState;

				tmpState->animation = loadFromAvo_LE_UINT16();
				tmpState->layer = loadFromAvo_LE_UINT16();
				_avoDataPtr += 8;
				tmpState->unk0 = loadFromAvo_LE_UINT16();
				tmpState->unk1 = loadFromAvo_LE_UINT16();

				_avoDataPtr += 2;
				if (READ_LE_UINT32(_avoDataPtr) != 0) {
					_avoDataPtr += 4;
					tmpState->sndItem = loadFromAvo_LE_UINT16();
				} else {
					_avoDataPtr += 6;
					tmpState->sndItem = -1;
				}
				tmpState->freq = loadFromAvo_LE_UINT16();
				tmpState->repCount = loadFromAvo_LE_UINT16();
				tmpState->sndFrame = loadFromAvo_LE_UINT16();
			}
		}
		delete[] tempstatedata;
	}

	_vm->_goblin->_objects[10] = new Goblin::Gob_Object;
	memset(_vm->_goblin->_objects[10], 0, sizeof(Goblin::Gob_Object));

	_vm->_goblin->_objects[10]->stateMach = new Goblin::Gob_StateLine[40];
	for (state = 0; state < 40; ++state)
		for (col = 0; col < 6; ++col)
			_vm->_goblin->_objects[10]->stateMach[state][col] = 0;

	pState = new Goblin::Gob_State;
	_vm->_goblin->_objects[10]->stateMach[0][0] = pState;

	memset(pState, 0, sizeof(Goblin::Gob_State));
	pState->animation = 9;
	pState->layer = 27;
	pState->unk0 = 0;
	pState->unk1 = 0;
	pState->sndItem = -1;
	pState->sndFrame = 0;

	_vm->_goblin->placeObject(_vm->_goblin->_objects[10], 1, 0, 0, 0, 0);

	_vm->_goblin->_objects[10]->realStateMach = _vm->_goblin->_objects[10]->stateMach;
	_vm->_goblin->_objects[10]->type = 1;
	_vm->_goblin->_objects[10]->unk14 = 1;

	state = loadFromAvo_LE_UINT16();
	for (i = 0; i < state; i++) {
		_avoDataPtr += 30;

		loadDataFromAvo((char *)&flag, 4);
		_avoDataPtr += 56;

		if (flag != 0)
			_avoDataPtr += 30;
	}

	loadDataFromAvo((char *)&tmp, 2);
	_avoDataPtr += 48;
	loadItemToObject();
	_avoDataPtr = savedPtr;

	for (i = 0; i < soundCount; i++) {
		loadDataFromAvo(buf, 14);
		strcat(buf, ".SND");
		strcpy(sndNames[i], buf);
	}

	delete[] dataBuf;

	_vm->_goblin->_soundData[14] = _vm->_snd->loadSoundData("diamant1.snd");

	for (i = 0; i < soundCount; i++) {
		handle = _vm->_dataio->openData(sndNames[i]);
		if (handle < 0)
			continue;

		_vm->_dataio->closeData(handle);
		_vm->_goblin->_soundData[i] = _vm->_snd->loadSoundData(sndNames[i]);
	}
}

void Map_v1::optimizePoints(int16 index, int16 x, int16 y) {
	int16 i;

	if (_nearestWayPoint < _nearestDest) {
		for (i = _nearestWayPoint; i <= _nearestDest; i++) {
			if (checkDirectPath(-1, _curGoblinX, _curGoblinY,
				_wayPoints[i].x, _wayPoints[i].y) == 1)
				_nearestWayPoint = i;
		}
	} else if (_nearestWayPoint > _nearestDest) {
		for (i = _nearestWayPoint; i >= _nearestDest; i--) {
			if (checkDirectPath(-1, _curGoblinX, _curGoblinY,
				_wayPoints[i].x, _wayPoints[i].y) == 1)
				_nearestWayPoint = i;
		}
	}
}

void Map_v1::findNearestToGob(int16 index) {
	int16 wayPoint = findNearestWayPoint(_curGoblinX, _curGoblinY);

	if (wayPoint != -1)
		_nearestWayPoint = wayPoint;
}

void Map_v1::findNearestToDest(int16 index) {
	int16 wayPoint = findNearestWayPoint(_destX, _destY);

	if (wayPoint != -1)
		_nearestDest = wayPoint;
}

} // End of namespace Gob
