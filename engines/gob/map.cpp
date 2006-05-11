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
#include "gob/video.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/inter.h"
#include "gob/goblin.h"
#include "gob/sound.h"
#include "gob/scenery.h"

namespace Gob {

Map::Map(GobEngine *vm) : _vm(vm) {
	int i;

	for (i = 0; i < kMapHeight; i++)
		for (int j = 0; j < kMapWidth; j++) {
			_passMap[i][j] = 0;
			_itemsMap[i][j] = 0;
		}
	for (i = 0; i < 40; i++) {
		_wayPoints[i].x = 0;
		_wayPoints[i].y = 0;
	}
	for (i = 0; i < 40; i++) {
		_itemPoses[i].x = 0;
		_itemPoses[i].y = 0;
		_itemPoses[i].orient = 0;
	}

	_nearestWayPoint = 0;
	_nearestDest = 0;
	_curGoblinX = 0;
	_curGoblinY = 0;
	_destX = 0;
	_destY = 0;
	_loadFromAvo = 0;
	_sourceFile[0] = 0;
	_avoDataPtr = 0;
}

void Map::placeItem(int16 x, int16 y, int16 id) {
	if ((_itemsMap[y][x] & 0xff00) != 0)
		_itemsMap[y][x] = (_itemsMap[y][x] & 0xff00) | id;
	else
		_itemsMap[y][x] = (_itemsMap[y][x] & 0x00ff) | (id << 8);
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

	if (_passMap[y0][x0] == 3 && (dir & kUp)) {
		if (_passMap[y0 - 1][x0] != 0)
			return kDirN;
	}

	if (_passMap[y0][x0] == 3 && (dir & kDown)) {
		if (_passMap[y0 + 1][x0] != 0)
			return kDirS;
	}

	if (_passMap[y0][x0] == 6 && (dir & kUp)) {
		if (_passMap[y0 - 1][x0] != 0)
			return kDirN;
	}

	if (_passMap[y0][x0] == 6 && (dir & kDown)) {
		if (_passMap[y0 + 1][x0] != 0)
			return kDirS;
	}

	if (dir == kLeft) {
		if (x0 - 1 >= 0 && _passMap[y0][x0 - 1] != 0)
			return kDirW;
		return 0;
	}

	if (dir == kRight) {
		if (x0 + 1 < kMapWidth && _passMap[y0][x0 + 1] != 0)
			return kDirE;
		return 0;
	}

	if (dir == kUp) {
		if (y0 - 1 >= 0 && _passMap[y0 - 1][x0] != 0)
			return kDirN;

		if (y0 - 1 >= 0 && x0 - 1 >= 0
		    && _passMap[y0 - 1][x0 - 1] != 0)
			return kDirNW;

		if (y0 - 1 >= 0 && x0 + 1 < kMapWidth
		    && _passMap[y0 - 1][x0 + 1] != 0)
			return kDirNE;

		return 0;
	}

	if (dir == kDown) {
		if (y0 + 1 < kMapHeight && _passMap[y0 + 1][x0] != 0)
			return kDirS;

		if (y0 + 1 < kMapHeight && x0 - 1 >= 0
		    && _passMap[y0 + 1][x0 - 1] != 0)
			return kDirSW;

		if (y0 + 1 < kMapHeight && x0 + 1 < kMapWidth
		    && _passMap[y0 + 1][x0 + 1] != 0)
			return kDirSE;

		return 0;
	}

	if (dir == (kRight | kUp)) {
		if (y0 - 1 >= 0 && x0 + 1 < kMapWidth
		    && _passMap[y0 - 1][x0 + 1] != 0)
			return kDirNE;

		if (y0 - 1 >= 0 && _passMap[y0 - 1][x0] != 0)
			return kDirN;

		if (x0 + 1 < kMapWidth && _passMap[y0][x0 + 1] != 0)
			return kDirE;

		return 0;
	}

	if (dir == (kRight | kDown)) {
		if (x0 + 1 < kMapWidth && y0 + 1 < kMapHeight
		    && _passMap[y0 + 1][x0 + 1] != 0)
			return kDirSE;

		if (y0 + 1 < kMapHeight && _passMap[y0 + 1][x0] != 0)
			return kDirS;

		if (x0 + 1 < kMapWidth && _passMap[y0][x0 + 1] != 0)
			return kDirE;

		return 0;
	}

	if (dir == (kLeft | kUp)) {
		if (x0 - 1 >= 0 && y0 - 1 >= 0
		    && _passMap[y0 - 1][x0 - 1] != 0)
			return kDirNW;

		if (y0 - 1 >= 0 && _passMap[y0 - 1][x0] != 0)
			return kDirN;

		if (x0 - 1 >= 0 && _passMap[y0][x0 - 1] != 0)
			return kDirW;

		return 0;
	}

	if (dir == (kLeft | kDown)) {
		if (x0 - 1 >= 0 && y0 + 1 < kMapHeight
		    && _passMap[y0 + 1][x0 - 1] != 0)
			return kDirSW;

		if (y0 + 1 < kMapHeight && _passMap[y0 + 1][x0] != 0)
			return kDirS;

		if (x0 - 1 >= 0 && _passMap[y0][x0 - 1] != 0)
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
		if (_wayPoints[i].x < 0 ||
				_wayPoints[i].x >= kMapWidth ||
				_wayPoints[i].y < 0 || _wayPoints[i].y >= kMapHeight)
			return -1;

		tmp = ABS(x - _wayPoints[i].x) + ABS(y - _wayPoints[i].y);

		if (tmp <= length) {
			lnearestWayPoint = i;
			length = tmp;
		}
	}

	return lnearestWayPoint;
}

void Map::findNearestToGob(void) {
	int16 wayPoint = findNearestWayPoint(_curGoblinX, _curGoblinY);

	if (wayPoint != -1)
		_nearestWayPoint = wayPoint;
}

void Map::findNearestToDest(void) {
	int16 wayPoint = findNearestWayPoint(_destX, _destY);

	if (wayPoint != -1)
		_nearestDest = wayPoint;
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
				curX = _wayPoints[i0].x;
				curY = _wayPoints[i0].y;
				i0--;
			} else if (i0 < i1) {
				curX = _wayPoints[i0].x;
				curY = _wayPoints[i0].y;
				i0++;
			} else if (i0 == i1) {
				curX = _wayPoints[i0].x;
				curY = _wayPoints[i0].y;
			}
		}
		if (i0 == i1 && _wayPoints[i0].x == x0
		    && _wayPoints[i0].y == y0) {
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

	if (_nearestWayPoint < _nearestDest) {
		for (i = _nearestWayPoint; i <= _nearestDest; i++) {
			if (checkDirectPath(_curGoblinX, _curGoblinY,
				_wayPoints[i].x, _wayPoints[i].y) == 1)
				_nearestWayPoint = i;
		}
	} else if (_nearestWayPoint > _nearestDest) {
		for (i = _nearestWayPoint; i >= _nearestDest; i--) {
			if (checkDirectPath(_curGoblinX, _curGoblinY,
				_wayPoints[i].x, _wayPoints[i].y) == 1)
				_nearestWayPoint = i;
		}
	}
}

void Map::loadDataFromAvo(char *dest, int16 size) {
	memcpy(dest, _avoDataPtr, size);
	_avoDataPtr += size;
}

uint16 Map::loadFromAvo_LE_UINT16() {
	uint16 tmp = READ_LE_UINT16(_avoDataPtr);
	_avoDataPtr += 2;
	return tmp;
}

void Map::loadItemToObject(void) {
	int16 flag;
	int16 count;
	int16 i;

	flag = loadFromAvo_LE_UINT16();
	if (flag == 0)
		return;

	_avoDataPtr += 1456;
	count = loadFromAvo_LE_UINT16();
	for (i = 0; i < count; i++) {
		_avoDataPtr += 20;
		_vm->_goblin->_itemToObject[i] = loadFromAvo_LE_UINT16();
		_avoDataPtr += 5;
	}
}

void Map::loadMapsInitGobs(void) {
	int16 layer;
	int16 i;

	if (_loadFromAvo == 0)
		error("load: Loading .pas/.pos files is not supported!");

	for (i = 0; i < 3; i++) {
		_vm->_goblin->nextLayer(_vm->_goblin->_goblins[i]);
	}

	for (i = 0; i < 3; i++) {

		layer =
		    _vm->_goblin->_goblins[i]->stateMach[_vm->_goblin->_goblins[i]->state][0]->layer;

		_vm->_scenery->updateAnim(layer, 0, _vm->_goblin->_goblins[i]->animation, 0,
		    _vm->_goblin->_goblins[i]->xPos, _vm->_goblin->_goblins[i]->yPos, 0);

		_vm->_goblin->_goblins[i]->yPos = (_vm->_goblin->_gobPositions[i].y + 1) * 6 -
		    (_vm->_scenery->_toRedrawBottom - _vm->_scenery->_animTop);

		_vm->_goblin->_goblins[i]->xPos = _vm->_goblin->_gobPositions[i].x * 12 -
		    (_vm->_scenery->_toRedrawLeft - _vm->_scenery->_animLeft);

		_vm->_goblin->_goblins[i]->order = _vm->_scenery->_toRedrawBottom / 24 + 3;
	}

	_vm->_goblin->_currentGoblin = 0;
	_vm->_goblin->_pressedMapX = _vm->_goblin->_gobPositions[0].x;
	_vm->_goblin->_pressedMapY = _vm->_goblin->_gobPositions[0].y;
	_vm->_goblin->_pathExistence = 0;

	_vm->_goblin->_goblins[0]->doAnim = 0;
	_vm->_goblin->_goblins[1]->doAnim = 1;
	_vm->_goblin->_goblins[2]->doAnim = 1;
}

}				// End of namespace Gob
