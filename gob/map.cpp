/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/map.h"
#include "gob/video.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/inter.h"
#include "gob/goblin.h"
#include "gob/sound.h"
#include "gob/scenery.h"

namespace Gob {

Map::Map(GobEngine *vm) : _vm(vm) {
	for (int i = 0; i < kMapHeight; i++)
		for (int j = 0; j < kMapWidth; j++) {
			passMap[i][j] = 0;
			itemsMap[i][j] = 0;
		}
	for (int i = 0; i < 40; i++) {
		wayPoints[i].x = 0;
		wayPoints[i].y = 0;
	}
	for (int i = 0; i < 40; i++) {
		itemPoses[i].x = 0;
		itemPoses[i].y = 0;
		itemPoses[i].orient = 0;
	}

	nearestWayPoint = 0;
	nearestDest = 0;
	curGoblinX = 0;
	curGoblinY = 0;
	destX = 0;
	destY = 0;
	loadFromAvo = 0;
	sourceFile[0] = 0;
	avoDataPtr = 0;
}

void Map::placeItem(int16 x, int16 y, int16 id) {
	if ((itemsMap[y][x] & 0xff00) != 0)
		itemsMap[y][x] = (itemsMap[y][x] & 0xff00) | id;
	else
		itemsMap[y][x] = (itemsMap[y][x] & 0x00ff) | (id << 8);
}

enum {
	kLeft  = (1 << 0),
	kUp    = (1 << 1),
	kRight = (1 << 2),
	kDown  = (1 << 3)
};

int16 Map::getDirection(int16 x0, int16 y0, int16 x1, int16 y1) {
	int16 dir = 0;

	if (x0 == x1 && y0 == y1)
		return 0;

	if (!(x1 >= 0 && x1 < kMapWidth && y1 >= 0 && y1 < kMapHeight))
		return 0;

	if (y1 > y0)
		dir |= kDown;
	else if (y1 < y0)
		dir |= kUp;

	if (x1 > x0)
		dir |= kRight;
	else if (x1 < x0)
		dir |= kLeft;

	if (passMap[y0][x0] == 3 && (dir & kUp)) {
		if (passMap[y0 - 1][x0] != 0)
			return kDirN;
	}

	if (passMap[y0][x0] == 3 && (dir & kDown)) {
		if (passMap[y0 + 1][x0] != 0)
			return kDirS;
	}

	if (passMap[y0][x0] == 6 && (dir & kUp)) {
		if (passMap[y0 - 1][x0] != 0)
			return kDirN;
	}

	if (passMap[y0][x0] == 6 && (dir & kDown)) {
		if (passMap[y0 + 1][x0] != 0)
			return kDirS;
	}

	if (dir == kLeft) {
		if (x0 - 1 >= 0 && passMap[y0][x0 - 1] != 0)
			return kDirW;
		return 0;
	}

	if (dir == kRight) {
		if (x0 + 1 < kMapWidth && passMap[y0][x0 + 1] != 0)
			return kDirE;
		return 0;
	}

	if (dir == kUp) {
		if (y0 - 1 >= 0 && passMap[y0 - 1][x0] != 0)
			return kDirN;

		if (y0 - 1 >= 0 && x0 - 1 >= 0
		    && passMap[y0 - 1][x0 - 1] != 0)
			return kDirNW;

		if (y0 - 1 >= 0 && x0 + 1 < kMapWidth
		    && passMap[y0 - 1][x0 + 1] != 0)
			return kDirNE;

		return 0;
	}

	if (dir == kDown) {
		if (y0 + 1 < kMapHeight && passMap[y0 + 1][x0] != 0)
			return kDirS;

		if (y0 + 1 < kMapHeight && x0 - 1 >= 0
		    && passMap[y0 + 1][x0 - 1] != 0)
			return kDirSW;

		if (y0 + 1 < kMapHeight && x0 + 1 < kMapWidth
		    && passMap[y0 + 1][x0 + 1] != 0)
			return kDirSE;

		return 0;
	}

	if (dir == (kRight | kUp)) {
		if (y0 - 1 >= 0 && x0 + 1 < kMapWidth
		    && passMap[y0 - 1][x0 + 1] != 0)
			return kDirNE;

		if (y0 - 1 >= 0 && passMap[y0 - 1][x0] != 0)
			return kDirN;

		if (x0 + 1 < kMapWidth && passMap[y0][x0 + 1] != 0)
			return kDirE;

		return 0;
	}

	if (dir == (kRight | kDown)) {
		if (x0 + 1 < kMapWidth && y0 + 1 < kMapHeight
		    && passMap[y0 + 1][x0 + 1] != 0)
			return kDirSE;

		if (y0 + 1 < kMapHeight && passMap[y0 + 1][x0] != 0)
			return kDirS;

		if (x0 + 1 < kMapWidth && passMap[y0][x0 + 1] != 0)
			return kDirE;

		return 0;
	}

	if (dir == (kLeft | kUp)) {
		if (x0 - 1 >= 0 && y0 - 1 >= 0
		    && passMap[y0 - 1][x0 - 1] != 0)
			return kDirNW;

		if (y0 - 1 >= 0 && passMap[y0 - 1][x0] != 0)
			return kDirN;

		if (x0 - 1 >= 0 && passMap[y0][x0 - 1] != 0)
			return kDirW;

		return 0;
	}

	if (dir == (kLeft | kDown)) {
		if (x0 - 1 >= 0 && y0 + 1 < kMapHeight
		    && passMap[y0 + 1][x0 - 1] != 0)
			return kDirSW;

		if (y0 + 1 < kMapHeight && passMap[y0 + 1][x0] != 0)
			return kDirS;

		if (x0 - 1 >= 0 && passMap[y0][x0 - 1] != 0)
			return kDirW;

		return 0;
	}
	return -1;
}

int16 Map::findNearestWayPoint(int16 x, int16 y) {
	int16 lnearestWayPoint = -1;
	int16 length;
	int16 i;
	int16 tmp;

	length = 30000;

	for (i = 0; i < 40; i++) {
		if (wayPoints[i].x < 0 ||
				wayPoints[i].x >= kMapWidth ||
				wayPoints[i].y < 0 || wayPoints[i].y >= kMapHeight)
			return -1;

		tmp = ABS(x - wayPoints[i].x) + ABS(y - wayPoints[i].y);

		if (tmp <= length) {
			lnearestWayPoint = i;
			length = tmp;
		}
	}

	return lnearestWayPoint;
}

void Map::findNearestToGob(void) {
	int16 wayPoint = findNearestWayPoint(curGoblinX, curGoblinY);

	if (wayPoint != -1)
		nearestWayPoint = wayPoint;
}

void Map::findNearestToDest(void) {
	int16 wayPoint = findNearestWayPoint(destX, destY);

	if (wayPoint != -1)
		nearestDest = wayPoint;
}

int16 Map::checkDirectPath(int16 x0, int16 y0, int16 x1, int16 y1) {
	uint16 dir;

	while (1) {
		dir = getDirection(x0, y0, x1, y1);

		if (x0 == x1 && y0 == y1)
			return 1;

		if (dir == 0)
			return 3;

		switch (dir) {
		case kDirNW:
			x0--;
			y0--;
			break;

		case kDirN:
			y0--;
			break;

		case kDirNE:
			x0++;
			y0--;
			break;

		case kDirW:
			x0--;
			break;

		case kDirE:
			x0++;
			break;

		case kDirSW:
			x0--;
			y0++;
			break;

		case kDirS:
			y0++;
			break;

		case kDirSE:
			x0++;
			y0++;
			break;
		}
	}
}

int16 Map::checkLongPath(int16 x0, int16 y0, int16 x1, int16 y1, int16 i0, int16 i1) {
	uint16 dir;
	int16 curX;
	int16 curY;
	int16 nextLink;

	curX = x0;
	curY = y0;
	dir = 0;

	nextLink = 1;

	while (1) {
		if (x0 == curX && y0 == curY)
			nextLink = 1;

		if (nextLink != 0) {
			if (checkDirectPath(x0, y0, x1, y1) == 1)
				return 1;

			nextLink = 0;
			if (i0 > i1) {
				curX = wayPoints[i0].x;
				curY = wayPoints[i0].y;
				i0--;
			} else if (i0 < i1) {
				curX = wayPoints[i0].x;
				curY = wayPoints[i0].y;
				i0++;
			} else if (i0 == i1) {
				curX = wayPoints[i0].x;
				curY = wayPoints[i0].y;
			}
		}
		if (i0 == i1 && wayPoints[i0].x == x0
		    && wayPoints[i0].y == y0) {
			if (checkDirectPath(x0, y0, x1, y1) == 1)
				return 1;
			return 0;
		}
		dir = getDirection(x0, y0, curX, curY);
		switch (dir) {
		case 0:
			return 0;

		case kDirNW:
			x0--;
			y0--;
			break;

		case kDirN:
			y0--;
			break;

		case kDirNE:
			x0++;
			y0--;
			break;

		case kDirW:
			x0--;
			break;

		case kDirE:
			x0++;
			break;

		case kDirSW:
			x0--;
			y0++;
			break;

		case kDirS:
			y0++;
			break;

		case kDirSE:
			x0++;
			y0++;
			break;
		}
	}
}

void Map::optimizePoints(void) {
	int16 i;

	if (nearestWayPoint < nearestDest) {
		for (i = nearestWayPoint; i <= nearestDest; i++) {
			if (checkDirectPath(curGoblinX, curGoblinY,
				wayPoints[i].x, wayPoints[i].y) == 1)
				nearestWayPoint = i;
		}
	} else if (nearestWayPoint > nearestDest) {
		for (i = nearestWayPoint; i >= nearestDest; i--) {
			if (checkDirectPath(curGoblinX, curGoblinY,
				wayPoints[i].x, wayPoints[i].y) == 1)
				nearestWayPoint = i;
		}
	}
}

void Map::loadDataFromAvo(char *dest, int16 size) {
	memcpy(dest, avoDataPtr, size);
	avoDataPtr += size;
}

uint16 Map::loadFromAvo_LE_UINT16() {
	uint16 tmp = READ_LE_UINT16(avoDataPtr);
	avoDataPtr += 2;
	return tmp;
}

void Map::loadItemToObject(void) {
	int16 flag;
	int16 count;
	int16 i;

	flag = loadFromAvo_LE_UINT16();
	if (flag == 0)
		return;

	avoDataPtr += 1456;
	count = loadFromAvo_LE_UINT16();
	for (i = 0; i < count; i++) {
		avoDataPtr += 20;
		_vm->_goblin->itemToObject[i] = loadFromAvo_LE_UINT16();
		avoDataPtr += 5;
	}
}

void Map::loadMapObjects(char *avjFile) {
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

	strcpy(avoName, sourceFile);
	strcat(avoName, ".avo");

	handle = _vm->_dataio->openData(avoName);
	if (handle >= 0) {
		loadFromAvo = 1;
		_vm->_dataio->closeData(handle);
		avoDataPtr = _vm->_dataio->getData(avoName);
		dataBuf = avoDataPtr;
		loadDataFromAvo((char *)passMap, kMapHeight * kMapWidth);

		for (y = 0; y < kMapHeight; y++) {
			for (x = 0; x < kMapWidth; x++) {
				loadDataFromAvo(&item, 1);
				itemsMap[y][x] = item;
			}
		}

		for (i = 0; i < 40; i++) {
			wayPoints[i].x = loadFromAvo_LE_UINT16();
			wayPoints[i].y = loadFromAvo_LE_UINT16();
		}
		loadDataFromAvo((char *)itemPoses, szMap_ItemPos * 20);
	} else {
		loadFromAvo = 0;
		avoDataPtr = _vm->_dataio->getData(avjFile);
		dataBuf = avoDataPtr;
	}

	avoDataPtr += 32;
	avoDataPtr += 76;
	avoDataPtr += 4;
	avoDataPtr += 20;

	for (i = 0; i < 3; i++) {
		tmp = loadFromAvo_LE_UINT16();
		avoDataPtr += tmp * 14;
	}

	soundCount = loadFromAvo_LE_UINT16();
	savedPtr = avoDataPtr;

	avoDataPtr += 14 * soundCount;
	avoDataPtr += 4;
	avoDataPtr += 24;

	count2 = loadFromAvo_LE_UINT16();
	count3 = loadFromAvo_LE_UINT16();

	savedPtr2 = avoDataPtr;
	avoDataPtr += count2 * 8;

	savedPtr3 = avoDataPtr;
	avoDataPtr += count3 * 8;

	_vm->_goblin->gobsCount = loadFromAvo_LE_UINT16();
	for (i = 0; i < _vm->_goblin->gobsCount; i++) {
		_vm->_goblin->goblins[i] = (Goblin::Gob_Object *)malloc(sizeof(Goblin::Gob_Object));

		_vm->_goblin->goblins[i]->xPos = READ_LE_UINT16(savedPtr2);
		savedPtr2 += 2;

		_vm->_goblin->goblins[i]->yPos = READ_LE_UINT16(savedPtr2);
		savedPtr2 += 2;

		_vm->_goblin->goblins[i]->order = READ_LE_UINT16(savedPtr2);
		savedPtr2 += 2;

		_vm->_goblin->goblins[i]->state = READ_LE_UINT16(savedPtr2);
		savedPtr2 += 2;

		if (i == 3)
			_vm->_goblin->goblins[i]->stateMach = (Goblin::Gob_StateLine *)malloc(szGob_StateLine * 70);
		else
			_vm->_goblin->goblins[i]->stateMach = (Goblin::Gob_StateLine *)malloc(szGob_StateLine * 40);

		// FIXME: All is wrong further. We should unwind calls to loadDataFromAvo()
		loadDataFromAvo((char *)_vm->_goblin->goblins[i]->stateMach, 40 * szGob_StateLine);
		avoDataPtr += 160;
		_vm->_goblin->goblins[i]->multObjIndex = *avoDataPtr;
		avoDataPtr += 2;

		_vm->_goblin->goblins[i]->realStateMach = _vm->_goblin->goblins[i]->stateMach;
		for (state = 0; state < 40; state++) {
			for (col = 0; col < 6; col++) {
				if (_vm->_goblin->goblins[i]->stateMach[state][col] == 0)
					continue;

				Goblin::Gob_State *tmpState = (Goblin::Gob_State *)malloc(sizeof(Goblin::Gob_State));
				_vm->_goblin->goblins[i]->stateMach[state][col] = tmpState;

				tmpState->animation = loadFromAvo_LE_UINT16();
				tmpState->layer = loadFromAvo_LE_UINT16();
				avoDataPtr += 8;
				tmpState->unk0 = loadFromAvo_LE_UINT16();
				tmpState->unk1 = loadFromAvo_LE_UINT16();

				avoDataPtr += 2;
				if (READ_LE_UINT32(avoDataPtr) != 0) {
					avoDataPtr += 4;
					tmpState->sndItem = loadFromAvo_LE_UINT16();
				} else {
					avoDataPtr += 6;
					tmpState->sndItem = -1;
				}
				tmpState->freq = loadFromAvo_LE_UINT16();
				tmpState->repCount = loadFromAvo_LE_UINT16();
				tmpState->sndFrame = loadFromAvo_LE_UINT16();
			}
		}
	}

	pState = (Goblin::Gob_State *)malloc(sizeof(Goblin::Gob_State));
	_vm->_goblin->goblins[0]->stateMach[39][0] = pState;
	pState->animation = 0;
	pState->layer = 98;
	pState->unk0 = 0;
	pState->unk1 = 0;
	pState->sndItem = -1;

	pState = (Goblin::Gob_State *) malloc(sizeof(Goblin::Gob_State));
	_vm->_goblin->goblins[1]->stateMach[39][0] = pState;
	pState->animation = 0;
	pState->layer = 99;
	pState->unk0 = 0;
	pState->unk1 = 0;
	pState->sndItem = -1;

	pState = (Goblin::Gob_State *) malloc(sizeof(Goblin::Gob_State));
	_vm->_goblin->goblins[2]->stateMach[39][0] = pState;
	pState->animation = 0;
	pState->layer = 100;
	pState->unk0 = 0;
	pState->unk1 = 0;
	pState->sndItem = -1;

	_vm->_goblin->goblins[2]->stateMach[10][0]->sndFrame = 13;
	_vm->_goblin->goblins[2]->stateMach[11][0]->sndFrame = 13;
	_vm->_goblin->goblins[2]->stateMach[28][0]->sndFrame = 13;
	_vm->_goblin->goblins[2]->stateMach[29][0]->sndFrame = 13;

	_vm->_goblin->goblins[1]->stateMach[10][0]->sndFrame = 13;
	_vm->_goblin->goblins[1]->stateMach[11][0]->sndFrame = 13;

	for (state = 40; state < 70; state++) {
		pState = (Goblin::Gob_State *)malloc(sizeof(Goblin::Gob_State));
		_vm->_goblin->goblins[3]->stateMach[state][0] = pState;
		_vm->_goblin->goblins[3]->stateMach[state][1] = 0;

		pState->animation = 9;
		pState->layer = state - 40;
		pState->sndItem = -1;
		pState->sndFrame = 0;
	}

	_vm->_goblin->objCount = loadFromAvo_LE_UINT16();
	for (i = 0; i < _vm->_goblin->objCount; i++) {
		_vm->_goblin->objects[i] =
		    (Goblin::Gob_Object *) malloc(sizeof(Goblin::Gob_Object));

		_vm->_goblin->objects[i]->xPos = READ_LE_UINT16(savedPtr3);
		savedPtr3 += 2;

		_vm->_goblin->objects[i]->yPos = READ_LE_UINT16(savedPtr3);
		savedPtr3 += 2;

		_vm->_goblin->objects[i]->order = READ_LE_UINT16(savedPtr3);
		savedPtr3 += 2;

		_vm->_goblin->objects[i]->state = READ_LE_UINT16(savedPtr3);
		savedPtr3 += 2;

		_vm->_goblin->objects[i]->stateMach = (Goblin::Gob_StateLine *)malloc(szGob_StateLine * 40);

		loadDataFromAvo((char *)_vm->_goblin->objects[i]->stateMach, 40 * szGob_StateLine);
		avoDataPtr += 160;
		_vm->_goblin->objects[i]->multObjIndex = *avoDataPtr;
		avoDataPtr += 2;

		_vm->_goblin->objects[i]->realStateMach = _vm->_goblin->objects[i]->stateMach;
		for (state = 0; state < 40; state++) {
			for (col = 0; col < 6; col++) {
				if (_vm->_goblin->objects[i]->stateMach[state][col] == 0)
					continue;

				Goblin::Gob_State *tmpState = (Goblin::Gob_State *)malloc(sizeof(Goblin::Gob_State));
				_vm->_goblin->objects[i]->stateMach[state][col] = tmpState;

				tmpState->animation = loadFromAvo_LE_UINT16();
				tmpState->layer = loadFromAvo_LE_UINT16();
				avoDataPtr += 8;
				tmpState->unk0 = loadFromAvo_LE_UINT16();
				tmpState->unk1 = loadFromAvo_LE_UINT16();

				avoDataPtr += 2;
				if (READ_LE_UINT32(avoDataPtr) != 0) {
					avoDataPtr += 4;
					tmpState->sndItem = loadFromAvo_LE_UINT16();
				} else {
					avoDataPtr += 6;
					tmpState->sndItem = -1;
				}
				tmpState->freq = loadFromAvo_LE_UINT16();
				tmpState->repCount = loadFromAvo_LE_UINT16();
				tmpState->sndFrame = loadFromAvo_LE_UINT16();
			}
		}
	}

	_vm->_goblin->objects[10] = (Goblin::Gob_Object *)malloc(sizeof(Goblin::Gob_Object));
	memset(_vm->_goblin->objects[10], 0, sizeof(Goblin::Gob_Object));

	_vm->_goblin->objects[10]->stateMach = (Goblin::Gob_StateLine *)malloc(szGob_StateLine * 40);
	memset(_vm->_goblin->objects[10]->stateMach, 0, szGob_StateLine * 40);

	pState = (Goblin::Gob_State *)malloc(sizeof(Goblin::Gob_State));
	_vm->_goblin->objects[10]->stateMach[0][0] = pState;

	memset(pState, 0, sizeof(Goblin::Gob_State));
	pState->animation = 9;
	pState->layer = 27;
	pState->unk0 = 0;
	pState->unk1 = 0;
	pState->sndItem = -1;
	pState->sndFrame = 0;

	_vm->_goblin->placeObject(_vm->_goblin->objects[10], 1);

	_vm->_goblin->objects[10]->realStateMach = _vm->_goblin->objects[10]->stateMach;
	_vm->_goblin->objects[10]->type = 1;
	_vm->_goblin->objects[10]->unk14 = 1;

	state = loadFromAvo_LE_UINT16();
	for (i = 0; i < state; i++) {
		avoDataPtr += 30;

		loadDataFromAvo((char *)&flag, 4);
		avoDataPtr += 56;

		if (flag != 0)
			avoDataPtr += 30;
	}

	loadDataFromAvo((char *)&tmp, 2);
	avoDataPtr += 48;
	loadItemToObject();
	avoDataPtr = savedPtr;

	for (i = 0; i < soundCount; i++) {
		loadDataFromAvo(buf, 14);
		strcat(buf, ".SND");
		strcpy(sndNames[i], buf);
	}

	free(dataBuf);

	_vm->_goblin->soundData[14] = _vm->_snd->loadSoundData("diamant1.snd");

	for (i = 0; i < soundCount; i++) {
		handle = _vm->_dataio->openData(sndNames[i]);
		if (handle < 0)
			continue;

		_vm->_dataio->closeData(handle);
		_vm->_goblin->soundData[i] = _vm->_snd->loadSoundData(sndNames[i]);
	}
}

void Map::loadMapsInitGobs(void) {
	int16 layer;
	int16 i;

	if (loadFromAvo == 0)
		error("load: Loading .pas/.pos files is not supported!");

	for (i = 0; i < 3; i++) {
		_vm->_goblin->nextLayer(_vm->_goblin->goblins[i]);
	}

	for (i = 0; i < 3; i++) {

		layer =
		    _vm->_goblin->goblins[i]->stateMach[_vm->_goblin->goblins[i]->state][0]->layer;

		_vm->_scenery->updateAnim(layer, 0, _vm->_goblin->goblins[i]->animation, 0,
		    _vm->_goblin->goblins[i]->xPos, _vm->_goblin->goblins[i]->yPos, 0);

		_vm->_goblin->goblins[i]->yPos = (_vm->_goblin->gobPositions[i].y + 1) * 6 -
		    (_vm->_scenery->toRedrawBottom - _vm->_scenery->animTop);

		_vm->_goblin->goblins[i]->xPos = _vm->_goblin->gobPositions[i].x * 12 -
		    (_vm->_scenery->toRedrawLeft - _vm->_scenery->animLeft);

		_vm->_goblin->goblins[i]->order = _vm->_scenery->toRedrawBottom / 24 + 3;
	}

	_vm->_goblin->currentGoblin = 0;
	_vm->_goblin->pressedMapX = _vm->_goblin->gobPositions[0].x;
	_vm->_goblin->pressedMapY = _vm->_goblin->gobPositions[0].y;
	_vm->_goblin->pathExistence = 0;

	_vm->_goblin->goblins[0]->doAnim = 0;
	_vm->_goblin->goblins[1]->doAnim = 1;
	_vm->_goblin->goblins[2]->doAnim = 1;
}

}				// End of namespace Gob
