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
 * $URL$
 * $Id$
 *
 */

#include "gob/gob.h"
#include "gob/map.h"
#include "gob/goblin.h"
#include "gob/scenery.h"
#include "gob/mult.h"

namespace Gob {

Map::Map(GobEngine *vm) : _vm(vm) {
	_widthByte = 0;
	_mapWidth = -1;
	_mapHeight = -1;
	_screenWidth = 0;
	_screenHeight = 0;
	_tilesWidth = 0;
	_tilesHeight = 0;
	_passWidth = 0;

	_passMap = 0;
	_itemsMap = 0;
	_wayPointsCount = 0;
	_wayPoints = 0;
	_bigTiles = false;

	for (int i = 0; i < 40; i++) {
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
	_sourceFile[0] = 0;

	_loadFromAvo = false;
}

Map::~Map() {
	delete[] _passMap;

	if (_itemsMap) {
		for (int i = 0; i < _mapHeight; i++)
			delete[] _itemsMap[i];
		delete[] _itemsMap;
	}

	delete[] _wayPoints;
}

void Map::placeItem(int16 x, int16 y, int16 id) {
	if ((getItem(x, y) & 0xFF00) != 0)
		setItem(x, y, (getItem(x, y) & 0xFF00) | id);
	else
		setItem(x, y, (getItem(x, y) & 0x00FF) | (id << 8));
}

enum {
	kLeft  = (1 << 0),
	kUp    = (1 << 1),
	kRight = (1 << 2),
	kDown  = (1 << 3)
};

int16 Map::getDirection(int16 x0, int16 y0, int16 x1, int16 y1) {
	int16 dir = 0;

	if ((x0 == x1) && (y0 == y1))
		return 0;

	if ((x1 < 0) || (x1 > _mapWidth) || (y1 < 0) || (y1 > _mapHeight))
		return 0;

	if (y1 > y0)
		dir |= kDown;
	else if (y1 < y0)
		dir |= kUp;

	if (x1 > x0)
		dir |= kRight;
	else if (x1 < x0)
		dir |= kLeft;

	if ((getPass(x0, y0) == 3) && (dir & kUp)) {
		if ((getPass(x0, y0 - 1) != 0))
			return kDirN;
	}

	if ((getPass(x0, y0) == 3) && (dir & kDown)) {
		if ((getPass(x0, y0 + 1) != 0))
			return kDirS;
	}

	if ((getPass(x0, y0) == 6) && (dir & kUp)) {
		if ((getPass(x0, y0 - 1) != 0))
			return kDirN;
	}

	if ((getPass(x0, y0) == 6) && (dir & kDown)) {
		if ((getPass(x0, y0 + 1) != 0))
			return kDirS;
	}

	if (dir == kLeft) {
		if (((x0 - 1) >= 0) && (getPass(x0 - 1, y0) != 0))
			return kDirW;
		return 0;
	}

	if (dir == kRight) {
		if (((x0 + 1) < _mapWidth) && (getPass(x0 + 1, y0) != 0))
			return kDirE;
		return 0;
	}

	if (dir == kUp) {
		if (((y0 - 1) >= 0) && (getPass(x0, y0 - 1) != 0))
			return kDirN;

		if (((y0 - 1) >= 0) && ((x0 - 1) >= 0) &&
		    (getPass(x0 - 1, y0 - 1) != 0))
			return kDirNW;

		if (((y0 - 1) >= 0) && ((x0 + 1) < _mapWidth) &&
		    (getPass(x0 + 1, y0 - 1) != 0))
			return kDirNE;

		return 0;
	}

	if (dir == kDown) {
		if (((y0 + 1) < _mapHeight) && (getPass(x0, y0 + 1) != 0))
			return kDirS;

		if (((y0 + 1) < _mapHeight) && ((x0 - 1) >= 0) &&
		    (getPass(x0 - 1, y0 + 1) != 0))
			return kDirSW;

		if (((y0 + 1) < _mapHeight) && ((x0 + 1) < _mapWidth) &&
		    (getPass(x0 + 1, y0 + 1) != 0))
			return kDirSE;

		return 0;
	}

	if (dir == (kRight | kUp)) {
		if (((y0 - 1) >= 0) && ((x0 + 1) < _mapWidth) &&
		    (getPass(x0 + 1, y0 - 1) != 0))
			return kDirNE;

		if (((y0 - 1) >= 0) && (getPass(x0, y0 - 1) != 0))
			return kDirN;

		if (((x0 + 1) < _mapWidth) && (getPass(x0 + 1, y0) != 0))
			return kDirE;

		return 0;
	}

	if (dir == (kRight | kDown)) {
		if (((x0 + 1) < _mapWidth) && ((y0 + 1) < _mapHeight) &&
		    (getPass(x0 + 1, y0 + 1) != 0))
			return kDirSE;

		if (((y0 + 1) < _mapHeight) && (getPass(x0, y0 + 1) != 0))
			return kDirS;

		if (((x0 + 1) < _mapWidth) && (getPass(x0 + 1, y0) != 0))
			return kDirE;

		return 0;
	}

	if (dir == (kLeft | kUp)) {
		if (((x0 - 1) >= 0) && ((y0 - 1) >= 0) &&
		    (getPass(x0 - 1, y0 - 1) != 0))
			return kDirNW;

		if (((y0 - 1) >= 0) && (getPass(x0, y0 - 1) != 0))
			return kDirN;

		if (((x0 - 1) >= 0) && (getPass(x0 - 1, y0) != 0))
			return kDirW;

		return 0;
	}

	if (dir == (kLeft | kDown)) {
		if (((x0 - 1) >= 0) && ((y0 + 1) < _mapHeight) &&
		    (getPass(x0 - 1, y0 + 1) != 0))
			return kDirSW;

		if (((y0 + 1) < _mapHeight) && (getPass(x0, y0 + 1) != 0))
			return kDirS;

		if (((x0 - 1) >= 0) && (getPass(x0 - 1, y0) != 0))
			return kDirW;

		return 0;
	}
	return -1;
}

int16 Map::findNearestWayPoint(int16 x, int16 y) {
	int16 nearestWayPoint = -1;
	int16 length;
	int16 tmp;

	length = 30000;

	for (int i = 0; i < _wayPointsCount; i++) {
		if ((_wayPoints[i].x < 0) || (_wayPoints[i].x >= _mapWidth) ||
				(_wayPoints[i].y < 0) || (_wayPoints[i].y >= _mapHeight))
			break;

		tmp = ABS(x - _wayPoints[i].x) + ABS(y - _wayPoints[i].y);

		if (tmp <= length) {
			nearestWayPoint = i;
			length = tmp;
		}
	}

	return nearestWayPoint;
}

void Map::findNearestWalkable(int16 &gobDestX, int16 &gobDestY,
		int16 mouseX, int16 mouseY) {
	int16 mapWidth, mapHeight;
	int16 pos1 = -1, pos2 = -1;
	int16 distance;
	int16 direction;
	int i;

	mapWidth = _screenWidth / _tilesWidth;
	mapHeight = _vm->_width / _tilesHeight;
	direction = 0;

	for (i = 1; i <= gobDestX; i++)
		if (getPass(gobDestX - i, gobDestY) != 0)
			break;
	if (i <= gobDestX)
		pos1 = ((i - 1) * _tilesWidth) + (mouseX % _tilesWidth) + 1;
	distance = i;

	for (i = 1; (gobDestX + i) < mapWidth; i++)
		if (getPass(gobDestX + i, gobDestY) != 0)
			break;
	if ((gobDestX + i) < mapWidth)
		pos2 = (i * _tilesWidth) - (mouseX % _tilesWidth);

	if ((pos2 != -1) && ((pos1 == -1) || (pos1 > pos2))) {
		pos1 = pos2;
		direction = 1;
		distance = i;
	}
	pos2 = -1;

	for (i = 1; (gobDestY + i) < mapHeight; i++)
		if (getPass(gobDestX, gobDestY + i) != 0)
			break;
	if ((gobDestY + i) < mapHeight)
		pos2 = (i * _tilesHeight) - (mouseY % _tilesHeight);

	if ((pos2 != -1) && ((pos1 == -1) || (pos1 > pos2))) {
		pos1 = pos2;
		direction = 2;
		distance = i;
	}
	pos2 = -1;

	for (i = 1; i <= gobDestY; i++)
		if (getPass(gobDestX, gobDestY - i) != 0)
			break;
	if (i <= gobDestY)
		pos2 = ((i - 1) * _tilesHeight) + (mouseY % _tilesHeight) + 1;

	if ((pos2 != -1) && ((pos1 == -1) || (pos1 > pos2))) {
		direction = 3;
		distance = i;
	}

	if (direction == 0)
		gobDestX -= distance;
	else if (direction == 1)
		gobDestX += distance;
	else if (direction == 2)
		gobDestY += distance;
	else if (direction == 3)
		gobDestY -= distance;
}

int16 Map::checkDirectPath(Mult::Mult_Object *obj,
		int16 x0, int16 y0, int16 x1, int16 y1) {
	uint16 dir;

	while (1) {
		dir = getDirection(x0, y0, x1, y1);

		if (obj) {
			if (obj->nearestWayPoint < obj->nearestDest) {
				if (_wayPoints[obj->nearestWayPoint + 1].notWalkable == 1)
					return 3;
			} else if (obj->nearestWayPoint > obj->nearestDest) {
				if (obj->nearestDest > 0)
					if (_wayPoints[obj->nearestDest - 1].notWalkable == 1)
						return 3;
			}
		}

		if ((x0 == x1) && (y0 == y1))
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

int16 Map::checkLongPath(int16 x0, int16 y0,
		int16 x1, int16 y1, int16 i0, int16 i1) {
	uint16 dir = 0;
	int16 curX = x0;
	int16 curY = y0;
	int16 nextLink = 1;

	while (1) {
		if ((x0 == curX) && (y0 == curY))
			nextLink = 1;

		if (nextLink != 0) {
			if (checkDirectPath(0, x0, y0, x1, y1) == 1)
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
		if ((i0 == i1) && (_wayPoints[i0].x == x0) &&
		    (_wayPoints[i0].y == y0)) {
			if (checkDirectPath(0, x0, y0, x1, y1) == 1)
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

void Map::loadMapsInitGobs() {
	int16 layer;

	if (!_loadFromAvo)
		error("Map::loadMapsInitGobs(): Loading .pas/.pos files is not supported");

	for (int i = 0; i < 3; i++)
		_vm->_goblin->nextLayer(_vm->_goblin->_goblins[i]);

	for (int i = 0; i < 3; i++) {
		Goblin::Gob_Object &gob = *_vm->_goblin->_goblins[i];

		layer = gob.stateMach[gob.state][0]->layer;
		_vm->_scenery->updateAnim(layer, 0, gob.animation, 0,
				gob.xPos, gob.yPos, 0);
		gob.yPos = (_vm->_goblin->_gobPositions[i].y + 1) * 6 -
			(_vm->_scenery->_toRedrawBottom - _vm->_scenery->_animTop);
		gob.xPos = _vm->_goblin->_gobPositions[i].x * 12 -
			(_vm->_scenery->_toRedrawLeft - _vm->_scenery->_animLeft);
		gob.order = _vm->_scenery->_toRedrawBottom / 24 + 3;
	}

	_vm->_goblin->_currentGoblin = 0;
	_vm->_goblin->_pressedMapX = _vm->_goblin->_gobPositions[0].x;
	_vm->_goblin->_pressedMapY = _vm->_goblin->_gobPositions[0].y;
	_vm->_goblin->_pathExistence = 0;

	_vm->_goblin->_goblins[0]->doAnim = 0;
	_vm->_goblin->_goblins[1]->doAnim = 1;
	_vm->_goblin->_goblins[2]->doAnim = 1;
}

} // End of namespace Gob
