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
 */

#include "gnap/gnap.h"
#include "gnap/datarchive.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"

namespace Gnap {

void GnapEngine::initSceneGrid(int gridMinX, int gridMinY, int gridMaxX, int gridMaxY) {
	_gridMinX = gridMinX;
	_gridMinY = gridMinY;
	_gridMaxX = gridMaxX;
	_gridMaxY = gridMaxY;
	_gnapGridX = 410 - gridMinX;
	_gnapGridY = 450 - gridMinY;
	_platGridX = 396 - gridMinX;
	_platGridY = 347 - gridMinY;
}

int GnapEngine::getGnapWalkSequenceId(int deltaX, int deltaY) {
	static const int _gnapWalkSequenceIds[9] = {
		0x7B2, 0x000, 0x7B4,
		0x7AD, 0x000, 0x7AE,
		0x7B1, 0x000, 0x7B3
	};
	// CHECKME This is a little weird
	return _gnapWalkSequenceIds[3 * deltaX + 3 + 1 + deltaY];
}

int GnapEngine::getGnapWalkStopSequenceId(int deltaX, int deltaY) {
	static const int _gnapWalkStopSequenceIds[9] = {
		0x7BC, 0x7BA, 0x7BA,
		0x7BC, 0x000, 0x7BA,
		0x7BB, 0x7B9, 0x7B9
	};
	// CHECKME This is a little weird
	return _gnapWalkStopSequenceIds[3 * deltaX + 3 + 1 + deltaY];
}

Facing GnapEngine::getGnapWalkFacing(int deltaX, int deltaY) {
	static const Facing _gnapWalkFacings[9] = {
		kDirUpLeft, kDirBottomLeft, kDirBottomLeft,
		kDirUpLeft, kDirNone, kDirBottomLeft,
		kDirUpRight, kDirBottomRight, kDirBottomRight
	};
	// CHECKME This is a little weird
	return _gnapWalkFacings[3 * deltaX + 3 + 1 + deltaY];
}

bool GnapEngine::isPointBlocked(int gridX, int gridY) {

	if (gridX < 0 || gridX >= _gridMaxX || gridY < 0 || gridY >= _gridMaxY)
		return true;

	if ((gridX == _gnapX && gridY == _gnapY) || (gridX == _platX && gridY == _platY))
		return true;
		
	const int x = _gridMinX + 75 * gridX;
	const int y = _gridMinY + 48 * gridY;

	for (int i = 0; i < _hotspotsCount; ++i) {
		if (x >= _hotspots[i]._x1 && x <= _hotspots[i]._x2 &&
			y >= _hotspots[i]._y1 && y <= _hotspots[i]._y2 &&
			!(_hotspots[i]._flags & SF_WALKABLE))
			return true;
	}

	return false;

}

bool GnapEngine::gridSub41F08B(int gridX, int gridY) {
	bool result = false;
	
	_gnapWalkNodesCount = 0;
	_gnapWalkDirXIncr = 0;
	_gnapWalkDirYIncr = 0;
	_gnapWalkDeltaX = ABS(_gnapWalkDestX - gridX);
	_gnapWalkDeltaY = ABS(_gnapWalkDestY - gridY);

	if (_gnapWalkDeltaX)
		_gnapWalkDirX = (_gnapWalkDestX - gridX) / _gnapWalkDeltaX;
	else
		_gnapWalkDirX = 0;

	if (_gnapWalkDeltaY)
		_gnapWalkDirY = (_gnapWalkDestY - gridY) / _gnapWalkDeltaY;
	else
		_gnapWalkDirY = 0;

	while (_gnapWalkDirXIncr < _gnapWalkDeltaX && _gnapWalkDirYIncr < _gnapWalkDeltaY) {
		_gnapWalkNodes[_gnapWalkNodesCount]._gridX1 = gridX + _gnapWalkDirX * _gnapWalkDirXIncr;
		_gnapWalkNodes[_gnapWalkNodesCount]._gridY1 = gridY + _gnapWalkDirY * _gnapWalkDirYIncr;
		if (!isPointBlocked(_gnapWalkDirX + _gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkDirY + _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = _gnapWalkDirX;
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = _gnapWalkDirY;
			++_gnapWalkDirXIncr;
			++_gnapWalkDirYIncr;
		} else if (_gnapWalkDeltaY - _gnapWalkDirYIncr > _gnapWalkDeltaX - _gnapWalkDirXIncr) {
			if (!isPointBlocked(_gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkDirY + _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = 0;
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = _gnapWalkDirY;
				++_gnapWalkDirYIncr;
			} else if (!isPointBlocked(_gnapWalkDirX + _gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = _gnapWalkDirX;
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = 0;
				++_gnapWalkDirXIncr;
			} else {
				_gnapWalkDeltaX = _gnapWalkDirXIncr;
				_gnapWalkDeltaY = _gnapWalkDirYIncr;
				--_gnapWalkNodesCount;
			}
		} else {
			if (!isPointBlocked(_gnapWalkDirX + _gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = _gnapWalkDirX;
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = 0;
				++_gnapWalkDirXIncr;
			} else if (!isPointBlocked(_gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkDirY + _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = 0;
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = _gnapWalkDirY;
				++_gnapWalkDirYIncr;
			} else {
				_gnapWalkDeltaX = _gnapWalkDirXIncr;
				_gnapWalkDeltaY = _gnapWalkDirYIncr;
				--_gnapWalkNodesCount;
			}
		}
		++_gnapWalkNodesCount;
	}
	
	while (_gnapWalkDirXIncr < _gnapWalkDeltaX) {
		_gnapWalkNodes[_gnapWalkNodesCount]._gridX1 = gridX + _gnapWalkDirX * _gnapWalkDirXIncr;
		_gnapWalkNodes[_gnapWalkNodesCount]._gridY1 = _gnapWalkDestY;
		if (!isPointBlocked(_gnapWalkDirX + _gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkDestY)) {
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = _gnapWalkDirX;
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = 0;
			++_gnapWalkDirXIncr;
			++_gnapWalkNodesCount;
		} else {
			_gnapWalkDeltaX = _gnapWalkDirXIncr;
		}
	}

	while (_gnapWalkDirYIncr < _gnapWalkDeltaY) {
		_gnapWalkNodes[_gnapWalkNodesCount]._gridX1 = _gnapWalkDestX;
		_gnapWalkNodes[_gnapWalkNodesCount]._gridY1 = gridY + _gnapWalkDirY * _gnapWalkDirYIncr;
		if (!isPointBlocked(_gnapWalkDestX, _gnapWalkDirY + _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = 0;
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = _gnapWalkDirY;
			++_gnapWalkDirYIncr;
			++_gnapWalkNodesCount;
		} else {
			_gnapWalkDeltaY = _gnapWalkDirYIncr;
		}
	}

	if (gridX + _gnapWalkDirX * _gnapWalkDirXIncr != _gnapWalkDestX || gridY + _gnapWalkDirY * _gnapWalkDirYIncr != _gnapWalkDestY) {
		_gnapWalkDestX = gridX + _gnapWalkDirX * _gnapWalkDirXIncr;
		_gnapWalkDestY = gridY + _gnapWalkDirY * _gnapWalkDirYIncr;
		result = false;
	} else {
		result = true;
	}

	return result;
}

bool GnapEngine::gridSub41F5FC(int gridX, int gridY, int index) {
	_gnapWalkNodesCount = index;
	_gnapWalkDirXIncr = 0;
	_gnapWalkDirYIncr = 0;
	_gnapWalkDeltaX = ABS(_gnapWalkDestX - gridX);
	_gnapWalkDeltaY = ABS(_gnapWalkDestY - gridY);

	if (_gnapWalkDeltaX)
		_gnapWalkDirX = (_gnapWalkDestX - gridX) / _gnapWalkDeltaX;
	else
		_gnapWalkDirX = 0;

	if (_gnapWalkDeltaY)
		_gnapWalkDirY = (_gnapWalkDestY - gridY) / _gnapWalkDeltaY;
	else
		_gnapWalkDirY = 0;

	while (_gnapWalkDirXIncr < _gnapWalkDeltaX && _gnapWalkDirYIncr < _gnapWalkDeltaY) {
		_gnapWalkNodes[_gnapWalkNodesCount]._gridX1 = gridX + _gnapWalkDirX * _gnapWalkDirXIncr;
		_gnapWalkNodes[_gnapWalkNodesCount]._gridY1 = gridY + _gnapWalkDirY * _gnapWalkDirYIncr;
		if (!isPointBlocked(_gnapWalkDirX + _gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkDirY + _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = _gnapWalkDirX;
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = _gnapWalkDirY;
			++_gnapWalkDirXIncr;
			++_gnapWalkDirYIncr;
		} else if (_gnapWalkDeltaY - _gnapWalkDirYIncr > _gnapWalkDeltaX - _gnapWalkDirXIncr) {
			if (!isPointBlocked(_gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkDirY + _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = 0;
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = _gnapWalkDirY;
				++_gnapWalkDirYIncr;
			} else if (!isPointBlocked(_gnapWalkDirX + _gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = _gnapWalkDirX;
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = 0;
				++_gnapWalkDirXIncr;
			} else
				return false;
		} else {
			if (!isPointBlocked(_gnapWalkDirX + _gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = _gnapWalkDirX;
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = 0;
				++_gnapWalkDirXIncr;
			} else if (!isPointBlocked(_gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkDirY + _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = 0;
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = _gnapWalkDirY;
				++_gnapWalkDirYIncr;
			} else
				return false;
		}
		++_gnapWalkNodesCount;
	}

	while (_gnapWalkDirXIncr < _gnapWalkDeltaX) {
		_gnapWalkNodes[_gnapWalkNodesCount]._gridX1 = gridX + _gnapWalkDirX * _gnapWalkDirXIncr;
		_gnapWalkNodes[_gnapWalkNodesCount]._gridY1 = _gnapWalkDestY;
		if (!isPointBlocked(_gnapWalkDirX + _gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkDestY)) {
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = _gnapWalkDirX;
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = 0;
			++_gnapWalkDirXIncr;
			++_gnapWalkNodesCount;
		} else
			return false;
	}

	while (_gnapWalkDirYIncr < _gnapWalkDeltaY) {
		_gnapWalkNodes[_gnapWalkNodesCount]._gridX1 = _gnapWalkDestX;
		_gnapWalkNodes[_gnapWalkNodesCount]._gridY1 = gridY + _gnapWalkDirY * _gnapWalkDirYIncr;
		if (!isPointBlocked(_gnapWalkDestX, _gnapWalkDirY + _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = 0;
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = _gnapWalkDirY;
			++_gnapWalkDirYIncr;
			++_gnapWalkNodesCount;
		} else
			return false;
	}

	return true;
}

bool GnapEngine::gridSub41FAD5(int gridX, int gridY, int index) {
	_gnapWalkNodesCount = index;
	_gnapWalkDirXIncr = 0;
	_gnapWalkDirYIncr = 0;
	_gnapWalkDeltaX = ABS(_gnapWalkDestX - gridX);
	_gnapWalkDeltaY = ABS(_gnapWalkDestY - gridY);

	if (_gnapWalkDeltaX)
		_gnapWalkDirX = (_gnapWalkDestX - gridX) / _gnapWalkDeltaX;
	else
		_gnapWalkDirX = 0;

	if (_gnapWalkDeltaY)
		_gnapWalkDirY = (_gnapWalkDestY - gridY) / _gnapWalkDeltaY;
	else
		_gnapWalkDirY = 0;

	while (_gnapWalkDeltaY < _gnapWalkDeltaX - _gnapWalkDirXIncr) {
		_gnapWalkNodes[_gnapWalkNodesCount]._gridX1 = gridX + _gnapWalkDirX * _gnapWalkDirXIncr;
		_gnapWalkNodes[_gnapWalkNodesCount]._gridY1 = gridY;
		if (!isPointBlocked(_gnapWalkDirX + _gnapWalkNodes[_gnapWalkNodesCount]._gridX1, gridY)) {
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = _gnapWalkDirX;
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = 0;
			++_gnapWalkDirXIncr;
			++_gnapWalkNodesCount;
		} else
			return false;
	}

	while (_gnapWalkDeltaX < _gnapWalkDeltaY - _gnapWalkDirYIncr) {
		_gnapWalkNodes[_gnapWalkNodesCount]._gridX1 = gridX;
		_gnapWalkNodes[_gnapWalkNodesCount]._gridY1 = gridY + _gnapWalkDirY * _gnapWalkDirYIncr;
		if (!isPointBlocked(gridX, _gnapWalkDirY + _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = 0;
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = _gnapWalkDirY;
			++_gnapWalkDirYIncr;
			++_gnapWalkNodesCount;
		} else
			return false;
	}

	while (_gnapWalkDirXIncr < _gnapWalkDeltaX && _gnapWalkDirYIncr < _gnapWalkDeltaY) {
		_gnapWalkNodes[_gnapWalkNodesCount]._gridX1 = gridX + _gnapWalkDirX * _gnapWalkDirXIncr;
		_gnapWalkNodes[_gnapWalkNodesCount]._gridY1 = gridY + _gnapWalkDirY * _gnapWalkDirYIncr;
		if (!isPointBlocked(_gnapWalkDirX + _gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkDirY + _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = _gnapWalkDirX;
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = _gnapWalkDirY;
			++_gnapWalkDirXIncr;
			++_gnapWalkDirYIncr;
		} else if (_gnapWalkDeltaY - _gnapWalkDirYIncr > _gnapWalkDeltaX - _gnapWalkDirXIncr) {
			if (!isPointBlocked(_gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkDirY + _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = 0;
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = _gnapWalkDirY;
				++_gnapWalkDirYIncr;
			} else if (!isPointBlocked(_gnapWalkDirX + _gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = _gnapWalkDirX;
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = 0;
				++_gnapWalkDirXIncr;
			} else
				return false;
		} else {
			if (!isPointBlocked(_gnapWalkDirX + _gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = _gnapWalkDirX;
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = 0;
				++_gnapWalkDirXIncr;
			} else if (!isPointBlocked(_gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkDirY + _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = 0;
				_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = _gnapWalkDirY;
				++_gnapWalkDirYIncr;
			} else
				return false;
		}
		++_gnapWalkNodesCount;
	}

	while (_gnapWalkDirXIncr < _gnapWalkDeltaX) {
		_gnapWalkNodes[_gnapWalkNodesCount]._gridX1 = gridX + _gnapWalkDirX * _gnapWalkDirXIncr;
		_gnapWalkNodes[_gnapWalkNodesCount]._gridY1 = _gnapWalkDestY;
		if (!isPointBlocked(_gnapWalkDirX + _gnapWalkNodes[_gnapWalkNodesCount]._gridX1, _gnapWalkDestY)) {
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = _gnapWalkDirX;
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = 0;
			++_gnapWalkDirXIncr;
			++_gnapWalkNodesCount;
		} else
			return false;
	}

	while (_gnapWalkDirYIncr < _gnapWalkDeltaY) {
		_gnapWalkNodes[_gnapWalkNodesCount]._gridX1 = _gnapWalkDestX;
		_gnapWalkNodes[_gnapWalkNodesCount]._gridY1 = gridY + _gnapWalkDirY * _gnapWalkDirYIncr;
		if (!isPointBlocked(_gnapWalkDestX, _gnapWalkDirY + _gnapWalkNodes[_gnapWalkNodesCount]._gridY1)) {
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaX = 0;
			_gnapWalkNodes[_gnapWalkNodesCount]._deltaY = _gnapWalkDirY;
			++_gnapWalkDirYIncr;
			++_gnapWalkNodesCount;
		} else
			return false;
	}

	return true;
}

bool GnapEngine::gnapFindPath3(int gridX, int gridY) {
	int gridIncr = 1;
	bool done = false;

	while (!done && gridIncr < _gridMaxX) {
		if (!isPointBlocked(gridX + gridIncr, gridY) && gridSub41F5FC(gridX + gridIncr, gridY, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX + i;
				_gnapWalkNodes[i]._gridY1 = gridY;
				_gnapWalkNodes[i]._deltaX = 1;
				_gnapWalkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX - gridIncr, gridY) && gridSub41F5FC(gridX - gridIncr, gridY, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX - i;
				_gnapWalkNodes[i]._gridY1 = gridY;
				_gnapWalkNodes[i]._deltaX = -1;
				_gnapWalkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX, gridY + gridIncr) && gridSub41F5FC(gridX, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX;
				_gnapWalkNodes[i]._gridY1 = gridY + i;
				_gnapWalkNodes[i]._deltaX = 0;
				_gnapWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX, gridY - gridIncr) && gridSub41F5FC(gridX, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX;
				_gnapWalkNodes[i]._gridY1 = gridY - i;
				_gnapWalkNodes[i]._deltaX = 0;
				_gnapWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX + gridIncr, gridY + gridIncr) && gridSub41F5FC(gridX + gridIncr, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX + i;
				_gnapWalkNodes[i]._gridY1 = gridY + i;
				_gnapWalkNodes[i]._deltaX = 1;
				_gnapWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX - gridIncr, gridY + gridIncr) && gridSub41F5FC(gridX - gridIncr, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX - i;
				_gnapWalkNodes[i]._gridY1 = gridY + i;
				_gnapWalkNodes[i]._deltaX = -1;
				_gnapWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX + gridIncr, gridY - gridIncr) && gridSub41F5FC(gridX + gridIncr, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX + i;
				_gnapWalkNodes[i]._gridY1 = gridY - i;
				_gnapWalkNodes[i]._deltaX = 1;
				_gnapWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX - gridIncr, gridY - gridIncr) && gridSub41F5FC(gridX - gridIncr, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX - i;
				_gnapWalkNodes[i]._gridY1 = gridY - i;
				_gnapWalkNodes[i]._deltaX = -1;
				_gnapWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX + gridIncr, gridY) && gridSub41FAD5(gridX + gridIncr, gridY, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX + i;
				_gnapWalkNodes[i]._gridY1 = gridY;
				_gnapWalkNodes[i]._deltaX = 1;
				_gnapWalkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX - gridIncr, gridY) && gridSub41FAD5(gridX - gridIncr, gridY, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX - i;
				_gnapWalkNodes[i]._gridY1 = gridY;
				_gnapWalkNodes[i]._deltaX = -1;
				_gnapWalkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX, gridY + gridIncr) && gridSub41FAD5(gridX, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX;
				_gnapWalkNodes[i]._gridY1 = gridY + i;
				_gnapWalkNodes[i]._deltaX = 0;
				_gnapWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX, gridY - gridIncr) && gridSub41FAD5(gridX, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX;
				_gnapWalkNodes[i]._gridY1 = gridY - i;
				_gnapWalkNodes[i]._deltaX = 0;
				_gnapWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX + gridIncr, gridY + gridIncr) && gridSub41FAD5(gridX + gridIncr, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX + i;
				_gnapWalkNodes[i]._gridY1 = gridY + i;
				_gnapWalkNodes[i]._deltaX = 1;
				_gnapWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX - gridIncr, gridY + gridIncr) && gridSub41FAD5(gridX - gridIncr, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX - i;
				_gnapWalkNodes[i]._gridY1 = gridY + i;
				_gnapWalkNodes[i]._deltaX = -1;
				_gnapWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX + gridIncr, gridY - gridIncr) && gridSub41FAD5(gridX + gridIncr, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX + i;
				_gnapWalkNodes[i]._gridY1 = gridY - i;
				_gnapWalkNodes[i]._deltaX = 1;
				_gnapWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX - gridIncr, gridY - gridIncr) && gridSub41FAD5(gridX - gridIncr, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX - i;
				_gnapWalkNodes[i]._gridY1 = gridY - i;
				_gnapWalkNodes[i]._deltaX = -1;
				_gnapWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		++gridIncr;
	}

	return done;
}

bool GnapEngine::gnapWalkTo(int gridX, int gridY, int animationIndex, int sequenceId, int flags) {

	int datNum = flags & 3;
	bool done = false;

	_timers[2] = 200;
	_timers[3] = 300;

	if (gridX < 0)
		gridX = (_leftClickMouseX - _gridMinX + 37) / 75;

	if (gridY < 0)
		gridY = (_leftClickMouseY - _gridMinY + 24) / 48;

	_gnapWalkDestX = CLIP(gridX, 0, _gridMaxX - 1);
	_gnapWalkDestY = CLIP(gridY, 0, _gridMaxY - 1);
	
	if (animationIndex >= 0 && _gnapWalkDestX == _platX && _gnapWalkDestY == _platY)
		platypusMakeRoom();

	if (gridSub41F5FC(_gnapX, _gnapY, 0))
		done = true;

	if (!done && gridSub41FAD5(_gnapX, _gnapY, 0))
		done = true;
		
	if (!done && gnapFindPath3(_gnapX, _gnapY))
		done = true;

	if (!done && gridSub41F08B(_gnapX, _gnapY))
		done = true;

	gnapIdle();

	int gnapSequenceId = _gnapSequenceId;
	int gnapId = _gnapId;
	int gnapSequenceDatNum = _gnapSequenceDatNum;

	debugC(kDebugBasic, "_gnapWalkNodesCount: %d", _gnapWalkNodesCount);

	for (int index = 0; index < _gnapWalkNodesCount; ++index) {
		_gnapWalkNodes[index]._id = index + 20 * _gnapWalkNodes[index]._gridY1;
		if (_gnapWalkNodes[index]._deltaX == 1 && _gnapWalkNodes[index]._deltaY == 0) {
			if (index % 2) {
				_gameSys->insertSequence(makeRid(datNum, 0x7AB), _gnapWalkNodes[index]._id,
					makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
					kSeqScale | kSeqSyncWait, 0, 75 * _gnapWalkNodes[index]._gridX1 - _gnapGridX, 48 * _gnapWalkNodes[index]._gridY1 - _gnapGridY);
				_gnapWalkNodes[index]._sequenceId = 0x7AB;
				gnapSequenceId = 0x7AB;
			} else {
				_gameSys->insertSequence(makeRid(datNum, 0x7AC), _gnapWalkNodes[index]._id,
					makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
					kSeqScale | kSeqSyncWait, 0, 75 * _gnapWalkNodes[index]._gridX1 - _gnapGridX, 48 * _gnapWalkNodes[index]._gridY1 - _gnapGridY);
				_gnapWalkNodes[index]._sequenceId = 0x7AC;
				gnapSequenceId = 0x7AC;
			}
		} else if (_gnapWalkNodes[index]._deltaX == -1 && _gnapWalkNodes[index]._deltaY == 0) {
			if (index % 2) {
				_gameSys->insertSequence(makeRid(datNum, 0x7AF), _gnapWalkNodes[index]._id,
					makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
					kSeqScale | kSeqSyncWait, 0, 75 * _gnapWalkNodes[index]._gridX1 - _gnapGridX, 48 * _gnapWalkNodes[index]._gridY1 - _gnapGridY);
				_gnapWalkNodes[index]._sequenceId = 0x7AF;
				gnapSequenceId = 0x7AF;
			} else {
				_gameSys->insertSequence(makeRid(datNum, 0x7B0), _gnapWalkNodes[index]._id,
					makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
					kSeqScale | kSeqSyncWait, 0, 75 * _gnapWalkNodes[index]._gridX1 - _gnapGridX, 48 * _gnapWalkNodes[index]._gridY1 - _gnapGridY);
				_gnapWalkNodes[index]._sequenceId = 0x7B0;
				gnapSequenceId = 0x7B0;
			}
		} else {
			if (_gnapWalkNodes[index]._deltaY == -1)
				_gnapWalkNodes[index]._id -= 10;
			else
				_gnapWalkNodes[index]._id += 10;
			int newSequenceId = getGnapWalkSequenceId(_gnapWalkNodes[index]._deltaX, _gnapWalkNodes[index]._deltaY);
			_gameSys->insertSequence(makeRid(datNum, newSequenceId), _gnapWalkNodes[index]._id,
				makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
				kSeqScale | kSeqSyncWait, 0, 75 * _gnapWalkNodes[index]._gridX1 - _gnapGridX, 48 * _gnapWalkNodes[index]._gridY1 - _gnapGridY);
			_gnapWalkNodes[index]._sequenceId = newSequenceId;
			gnapSequenceId = newSequenceId;
		}
		gnapId = _gnapWalkNodes[index]._id;
		gnapSequenceDatNum = datNum;
	}

	if (flags & 8) {
		if (_gnapWalkNodesCount > 0) {
			_gnapSequenceId = gnapSequenceId;
			_gnapId = gnapId;
			_gnapIdleFacing = getGnapWalkFacing(_gnapWalkNodes[_gnapWalkNodesCount - 1]._deltaX, _gnapWalkNodes[_gnapWalkNodesCount - 1]._deltaY);
			_gnapSequenceDatNum = datNum;
			if (animationIndex >= 0)
				_gameSys->setAnimation(makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, animationIndex);
		} else if (animationIndex >= 0) {
			_gameSys->setAnimation(0x107D3, 1, animationIndex);
			_gameSys->insertSequence(0x107D3, 1, 0, 0, kSeqNone, 0, 0, 0);
		}
	} else {
		if (sequenceId >= 0 && sequenceId != -1) {
			_gnapSequenceId = ridToEntryIndex(sequenceId);
			_gnapSequenceDatNum = ridToDatIndex(sequenceId);
			if (_gnapSequenceId == 0x7B9) {
				_gnapIdleFacing = kDirBottomRight;
			} else {
				switch (_gnapSequenceId) {
				case 0x7BA:
					_gnapIdleFacing = kDirBottomLeft;
					break;
				case 0x7BB:
					_gnapIdleFacing = kDirUpRight;
					break;
				case 0x7BC:
					_gnapIdleFacing = kDirUpLeft;
					break;
				}
			}
		} else {
			if (_gnapWalkNodesCount > 0) {
				_gnapSequenceId = getGnapWalkStopSequenceId(_gnapWalkNodes[_gnapWalkNodesCount - 1]._deltaX, _gnapWalkNodes[_gnapWalkNodesCount - 1]._deltaY);
				_gnapIdleFacing = getGnapWalkFacing(_gnapWalkNodes[_gnapWalkNodesCount - 1]._deltaX, _gnapWalkNodes[_gnapWalkNodesCount - 1]._deltaY);
			} else if (gridX >= 0 || gridY >= 0) {
				switch (_gnapIdleFacing) {
				case kDirBottomRight:
					_gnapSequenceId = 0x7B9;
					break;
				case kDirBottomLeft:
					_gnapSequenceId = 0x7BA;
					break;
				case kDirUpRight:
					_gnapSequenceId = 0x7BB;
					break;
				default:
					_gnapSequenceId = 0x7BC;
					break;
				}
			} else {
				int v10 = _leftClickMouseX - (_gridMinX + 75 * _gnapX);
				int v11 = _leftClickMouseY - (_gridMinY + 48 * _gnapY);
				if (_leftClickMouseX == _gridMinX + 75 * _gnapX)
					++v10;
				if (_leftClickMouseY == _gridMinY + 48 * _gnapY)
					v11 = 1;
				_gnapSequenceId = getGnapWalkStopSequenceId(v10 / abs(v10), v11 / abs(v11));
				_gnapIdleFacing = getGnapWalkFacing(v10 / abs(v10), v11 / abs(v11));
			}
			_gnapSequenceDatNum = datNum;
		}

		if (animationIndex < 0) {
			_gnapId = 20 * _gnapWalkDestY + 1;
		} else {
			_gnapId = _gnapWalkNodesCount + animationIndex + 20 * _gnapWalkDestY;
			_gameSys->setAnimation(makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapWalkNodesCount + animationIndex + 20 * _gnapWalkDestY, animationIndex);
		}

		if (flags & 4) {
			_gameSys->insertSequence(makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
				makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
				kSeqScale | kSeqSyncWait, 0, 0, 0);
		} else {
			_gameSys->insertSequence(makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
				makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
				kSeqScale | kSeqSyncWait, 0, 75 * _gnapWalkDestX - _gnapGridX, 48 * _gnapWalkDestY - _gnapGridY);
		}
	}

	_gnapX = _gnapWalkDestX;
	_gnapY = _gnapWalkDestY;

	return done;
}

void GnapEngine::gnapWalkStep() {
	bool done = false;
	for (int i = 1; i < _gridMaxX && !done; ++i) {
		done = true;
		if (!isPointBlocked(_gnapX + i, _gnapY))
			gnapWalkTo(_gnapX + i, _gnapY, -1, -1, 1);
		else if (!isPointBlocked(_gnapX - i, _gnapY))
			gnapWalkTo(_gnapX - i, _gnapY, -1, -1, 1);
		else if (!isPointBlocked(_gnapX, _gnapY + 1))
			gnapWalkTo(_gnapX, _gnapY + 1, -1, -1, 1);
		else if (!isPointBlocked(_gnapX, _gnapY - 1))
			gnapWalkTo(_gnapX, _gnapY - 1, -1, -1, 1);
		else if (!isPointBlocked(_gnapX + 1, _gnapY + 1))
			gnapWalkTo(_gnapX + 1, _gnapY + 1, -1, -1, 1);
		else if (!isPointBlocked(_gnapX - 1, _gnapY + 1))
			gnapWalkTo(_gnapX - 1, _gnapY + 1, -1, -1, 1);
		else if (!isPointBlocked(_gnapX + 1, _gnapY - 1))
			gnapWalkTo(_gnapX + 1, _gnapY - 1, -1, -1, 1);
		else if (!isPointBlocked(_gnapX - 1, _gnapY - 1))
			gnapWalkTo(_gnapX - 1, _gnapY - 1, -1, -1, 1);
		else
			done = false;
	}
}

////////////////////////////////////////////////////////////////////////////////

int GnapEngine::getPlatypusWalkSequenceId(int deltaX, int deltaY) {
	static const int _platypusWalkSequenceIds[9] = {
		0x7C5, 0x000, 0x7C8,
		0x7C4, 0x000, 0x7C7,
		0x7C3, 0x000, 0x7C6
	};
	// CHECKME This is a little weird
	return _platypusWalkSequenceIds[3 * deltaX + 3 + 1 + deltaY];
}

bool GnapEngine::gridSub423750(int gridX, int gridY) {
	bool result = false;
	
	_platWalkNodesCount = 0;
	_platWalkDirXIncr = 0;
	_platWalkDirYIncr = 0;
	_platWalkDeltaX = ABS(_platWalkDestX - gridX);
	_platWalkDeltaY = ABS(_platWalkDestY - gridY);

	if (_platWalkDeltaX)
		_platWalkDirX = (_platWalkDestX - gridX) / _platWalkDeltaX;
	else
		_platWalkDirX = 0;

	if (_platWalkDeltaY)
		_platWalkDirY = (_platWalkDestY - gridY) / _platWalkDeltaY;
	else
		_platWalkDirY = 0;

	while (_platWalkDirXIncr < _platWalkDeltaX && _platWalkDirYIncr < _platWalkDeltaY) {
		_platWalkNodes[_platWalkNodesCount]._gridX1 = gridX + _platWalkDirX * _platWalkDirXIncr;
		_platWalkNodes[_platWalkNodesCount]._gridY1 = gridY + _platWalkDirY * _platWalkDirYIncr;
		if (!isPointBlocked(_platWalkDirX + _platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkDirY + _platWalkNodes[_platWalkNodesCount]._gridY1)) {
			_platWalkNodes[_platWalkNodesCount]._deltaX = _platWalkDirX;
			_platWalkNodes[_platWalkNodesCount]._deltaY = _platWalkDirY;
			++_platWalkDirXIncr;
			++_platWalkDirYIncr;
		} else if (_platWalkDeltaY - _platWalkDirYIncr > _platWalkDeltaX - _platWalkDirXIncr) {
			if (!isPointBlocked(_platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkDirY + _platWalkNodes[_platWalkNodesCount]._gridY1)) {
				_platWalkNodes[_platWalkNodesCount]._deltaX = 0;
				_platWalkNodes[_platWalkNodesCount]._deltaY = _platWalkDirY;
				++_platWalkDirYIncr;
			} else if (!isPointBlocked(_platWalkDirX + _platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkNodes[_platWalkNodesCount]._gridY1)) {
				_platWalkNodes[_platWalkNodesCount]._deltaX = _platWalkDirX;
				_platWalkNodes[_platWalkNodesCount]._deltaY = 0;
				++_platWalkDirXIncr;
			} else {
				_platWalkDeltaX = _platWalkDirXIncr;
				_platWalkDeltaY = _platWalkDirYIncr;
				--_platWalkNodesCount;
			}
		} else {
			if (!isPointBlocked(_platWalkDirX + _platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkNodes[_platWalkNodesCount]._gridY1)) {
				_platWalkNodes[_platWalkNodesCount]._deltaX = _platWalkDirX;
				_platWalkNodes[_platWalkNodesCount]._deltaY = 0;
				++_platWalkDirXIncr;
			} else if (!isPointBlocked(_platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkDirY + _platWalkNodes[_platWalkNodesCount]._gridY1)) {
				_platWalkNodes[_platWalkNodesCount]._deltaX = 0;
				_platWalkNodes[_platWalkNodesCount]._deltaY = _platWalkDirY;
				++_platWalkDirYIncr;
			} else {
				_platWalkDeltaX = _platWalkDirXIncr;
				_platWalkDeltaY = _platWalkDirYIncr;
				--_platWalkNodesCount;
			}
		}
		++_platWalkNodesCount;
	}

	while (_platWalkDirXIncr < _platWalkDeltaX) {
		_platWalkNodes[_platWalkNodesCount]._gridX1 = gridX + _platWalkDirX * _platWalkDirXIncr;
		_platWalkNodes[_platWalkNodesCount]._gridY1 = _platWalkDestY;
		if (!isPointBlocked(_platWalkDirX + _platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkDestY)) {
			_platWalkNodes[_platWalkNodesCount]._deltaX = _platWalkDirX;
			_platWalkNodes[_platWalkNodesCount]._deltaY = 0;
			++_platWalkDirXIncr;
			++_platWalkNodesCount;
		} else {
			_platWalkDeltaX = _platWalkDirXIncr;
		}
	}

	while (_platWalkDirYIncr < _platWalkDeltaY) {
		_platWalkNodes[_platWalkNodesCount]._gridX1 = _platWalkDestX;
		_platWalkNodes[_platWalkNodesCount]._gridY1 = gridY + _platWalkDirY * _platWalkDirYIncr;
		if (!isPointBlocked(_platWalkDestX, _platWalkDirY + _platWalkNodes[_platWalkNodesCount]._gridY1)) {
			_platWalkNodes[_platWalkNodesCount]._deltaX = 0;
			_platWalkNodes[_platWalkNodesCount]._deltaY = _platWalkDirY;
			++_platWalkDirYIncr;
			++_platWalkNodesCount;
		} else {
			_platWalkDeltaY = _platWalkDirYIncr;
		}
	}

	if (gridX + _platWalkDirX * _platWalkDirXIncr != _platWalkDestX || gridY + _platWalkDirY * _platWalkDirYIncr != _platWalkDestY) {
		_platWalkDestX = gridX + _platWalkDirX * _platWalkDirXIncr;
		_platWalkDestY = gridY + _platWalkDirY * _platWalkDirYIncr;
		result = false;
	} else {
		result = true;
	}

	return result;
}

bool GnapEngine::gridSub423CC1(int gridX, int gridY, int index) {
	_platWalkNodesCount = index;
	_platWalkDirXIncr = 0;
	_platWalkDirYIncr = 0;
	_platWalkDeltaX = ABS(_platWalkDestX - gridX);
	_platWalkDeltaY = ABS(_platWalkDestY - gridY);

	if (_platWalkDeltaX)
		_platWalkDirX = (_platWalkDestX - gridX) / _platWalkDeltaX;
	else
		_platWalkDirX = 0;

	if (_platWalkDeltaY)
		_platWalkDirY = (_platWalkDestY - gridY) / _platWalkDeltaY;
	else
		_platWalkDirY = 0;

	while (_platWalkDirXIncr < _platWalkDeltaX && _platWalkDirYIncr < _platWalkDeltaY) {
		_platWalkNodes[_platWalkNodesCount]._gridX1 = gridX + _platWalkDirX * _platWalkDirXIncr;
		_platWalkNodes[_platWalkNodesCount]._gridY1 = gridY + _platWalkDirY * _platWalkDirYIncr;
		if (!isPointBlocked(_platWalkDirX + _platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkDirY + _platWalkNodes[_platWalkNodesCount]._gridY1)) {
			_platWalkNodes[_platWalkNodesCount]._deltaX = _platWalkDirX;
			_platWalkNodes[_platWalkNodesCount]._deltaY = _platWalkDirY;
			++_platWalkDirXIncr;
			++_platWalkDirYIncr;
		} else if (_platWalkDeltaY - _platWalkDirYIncr > _platWalkDeltaX - _platWalkDirXIncr) {
			if (!isPointBlocked(_platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkDirY + _platWalkNodes[_platWalkNodesCount]._gridY1)) {
				_platWalkNodes[_platWalkNodesCount]._deltaX = 0;
				_platWalkNodes[_platWalkNodesCount]._deltaY = _platWalkDirY;
				++_platWalkDirYIncr;
			} else if (!isPointBlocked(_platWalkDirX + _platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkNodes[_platWalkNodesCount]._gridY1)) {
				_platWalkNodes[_platWalkNodesCount]._deltaX = _platWalkDirX;
				_platWalkNodes[_platWalkNodesCount]._deltaY = 0;
				++_platWalkDirXIncr;
			} else
				return false;
		} else {
			if (!isPointBlocked(_platWalkDirX + _platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkNodes[_platWalkNodesCount]._gridY1)) {
				_platWalkNodes[_platWalkNodesCount]._deltaX = _platWalkDirX;
				_platWalkNodes[_platWalkNodesCount]._deltaY = 0;
				++_platWalkDirXIncr;
			} else if (!isPointBlocked(_platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkDirY + _platWalkNodes[_platWalkNodesCount]._gridY1)) {
				_platWalkNodes[_platWalkNodesCount]._deltaX = 0;
				_platWalkNodes[_platWalkNodesCount]._deltaY = _platWalkDirY;
				++_platWalkDirYIncr;
			} else
				return false;
		}
		++_platWalkNodesCount;
	}

	while (_platWalkDirXIncr < _platWalkDeltaX) {
		_platWalkNodes[_platWalkNodesCount]._gridX1 = gridX + _platWalkDirX * _platWalkDirXIncr;
		_platWalkNodes[_platWalkNodesCount]._gridY1 = _platWalkDestY;
		if (!isPointBlocked(_platWalkDirX + _platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkDestY)) {
			_platWalkNodes[_platWalkNodesCount]._deltaX = _platWalkDirX;
			_platWalkNodes[_platWalkNodesCount]._deltaY = 0;
			++_platWalkDirXIncr;
			++_platWalkNodesCount;
		} else
			return false;
	}

	while (_platWalkDirYIncr < _platWalkDeltaY) {
		_platWalkNodes[_platWalkNodesCount]._gridX1 = _platWalkDestX;
		_platWalkNodes[_platWalkNodesCount]._gridY1 = gridY + _platWalkDirY * _platWalkDirYIncr;
		if (!isPointBlocked(_platWalkDestX, _platWalkDirY + _platWalkNodes[_platWalkNodesCount]._gridY1)) {
			_platWalkNodes[_platWalkNodesCount]._deltaX = 0;
			_platWalkNodes[_platWalkNodesCount]._deltaY = _platWalkDirY;
			++_platWalkDirYIncr;
			++_platWalkNodesCount;
		} else
			return false;
	}

	return true;
}

bool GnapEngine::gridSub42419A(int gridX, int gridY, int index) {
	_platWalkNodesCount = index;
	_platWalkDirXIncr = 0;
	_platWalkDirYIncr = 0;
	_platWalkDeltaX = ABS(_platWalkDestX - gridX);
	_platWalkDeltaY = ABS(_platWalkDestY - gridY);

	if (_platWalkDeltaX)
		_platWalkDirX = (_platWalkDestX - gridX) / _platWalkDeltaX;
	else
		_platWalkDirX = 0;

	if (_platWalkDeltaY)
		_platWalkDirY = (_platWalkDestY - gridY) / _platWalkDeltaY;
	else
		_platWalkDirY = 0;

	while (_platWalkDeltaY < _platWalkDeltaX - _platWalkDirXIncr) {
		_platWalkNodes[_platWalkNodesCount]._gridX1 = gridX + _platWalkDirX * _platWalkDirXIncr;
		_platWalkNodes[_platWalkNodesCount]._gridY1 = gridY;
		if (!isPointBlocked(_platWalkDirX + _platWalkNodes[_platWalkNodesCount]._gridX1, gridY)) {
			_platWalkNodes[_platWalkNodesCount]._deltaX = _platWalkDirX;
			_platWalkNodes[_platWalkNodesCount]._deltaY = 0;
			++_platWalkDirXIncr;
			++_platWalkNodesCount;
		} else
			return false;
	}

	while (_platWalkDeltaX < _platWalkDeltaY - _platWalkDirYIncr) {
		_platWalkNodes[_platWalkNodesCount]._gridX1 = gridX;
		_platWalkNodes[_platWalkNodesCount]._gridY1 = gridY + _platWalkDirY * _platWalkDirYIncr;
		if (!isPointBlocked(gridX, _platWalkDirY + _platWalkNodes[_platWalkNodesCount]._gridY1)) {
			_platWalkNodes[_platWalkNodesCount]._deltaX = 0;
			_platWalkNodes[_platWalkNodesCount]._deltaY = _platWalkDirY;
			++_platWalkDirYIncr;
			++_platWalkNodesCount;
		} else
			return false;
	}

	while (_platWalkDirXIncr < _platWalkDeltaX && _platWalkDirYIncr < _platWalkDeltaY) {
		_platWalkNodes[_platWalkNodesCount]._gridX1 = gridX + _platWalkDirX * _platWalkDirXIncr;
		_platWalkNodes[_platWalkNodesCount]._gridY1 = gridY + _platWalkDirY * _platWalkDirYIncr;
		if (!isPointBlocked(_platWalkDirX + _platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkDirY + _platWalkNodes[_platWalkNodesCount]._gridY1)) {
			_platWalkNodes[_platWalkNodesCount]._deltaX = _platWalkDirX;
			_platWalkNodes[_platWalkNodesCount]._deltaY = _platWalkDirY;
			++_platWalkDirXIncr;
			++_platWalkDirYIncr;
		} else if (_platWalkDeltaY - _platWalkDirYIncr > _platWalkDeltaX - _platWalkDirXIncr) {
			if (!isPointBlocked(_platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkDirY + _platWalkNodes[_platWalkNodesCount]._gridY1)) {
				_platWalkNodes[_platWalkNodesCount]._deltaX = 0;
				_platWalkNodes[_platWalkNodesCount]._deltaY = _platWalkDirY;
				++_platWalkDirYIncr;
			} else if (!isPointBlocked(_platWalkDirX + _platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkNodes[_platWalkNodesCount]._gridY1)) {
				_platWalkNodes[_platWalkNodesCount]._deltaX = _platWalkDirX;
				_platWalkNodes[_platWalkNodesCount]._deltaY = 0;
				++_platWalkDirXIncr;
			} else
				return false;
		} else {
			if (!isPointBlocked(_platWalkDirX + _platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkNodes[_platWalkNodesCount]._gridY1)) {
				_platWalkNodes[_platWalkNodesCount]._deltaX = _platWalkDirX;
				_platWalkNodes[_platWalkNodesCount]._deltaY = 0;
				++_platWalkDirXIncr;
			} else if (!isPointBlocked(_platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkDirY + _platWalkNodes[_platWalkNodesCount]._gridY1)) {
				_platWalkNodes[_platWalkNodesCount]._deltaX = 0;
				_platWalkNodes[_platWalkNodesCount]._deltaY = _platWalkDirY;
				++_platWalkDirYIncr;
			} else
				return false;
		}
		++_platWalkNodesCount;
	}

	while (_platWalkDirXIncr < _platWalkDeltaX) {
		_platWalkNodes[_platWalkNodesCount]._gridX1 = gridX + _platWalkDirX * _platWalkDirXIncr;
		_platWalkNodes[_platWalkNodesCount]._gridY1 = _platWalkDestY;
		if (!isPointBlocked(_platWalkDirX + _platWalkNodes[_platWalkNodesCount]._gridX1, _platWalkDestY)) {
			_platWalkNodes[_platWalkNodesCount]._deltaX = _platWalkDirX;
			_platWalkNodes[_platWalkNodesCount]._deltaY = 0;
			++_platWalkDirXIncr;
			++_platWalkNodesCount;
		} else
			return false;
	}

	while (_platWalkDirYIncr < _platWalkDeltaY) {
		_platWalkNodes[_platWalkNodesCount]._gridX1 = _platWalkDestX;
		_platWalkNodes[_platWalkNodesCount]._gridY1 = gridY + _platWalkDirY * _platWalkDirYIncr;
		if (!isPointBlocked(_platWalkDestX, _platWalkDirY + _platWalkNodes[_platWalkNodesCount]._gridY1)) {
			_platWalkNodes[_platWalkNodesCount]._deltaX = 0;
			_platWalkNodes[_platWalkNodesCount]._deltaY = _platWalkDirY;
			++_platWalkDirYIncr;
			++_platWalkNodesCount;
		} else
			return false;
	}

	return true;
}

bool GnapEngine::platFindPath3(int gridX, int gridY) {
	int gridIncr = 1;
	bool done = false;

	while (!done && gridIncr < _gridMaxX) {
		if (!isPointBlocked(_platX + gridIncr, _platY) && gridSub423CC1(_platX + gridIncr, _platY, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _platX + i;
				_platWalkNodes[i]._gridY1 = _platY;
				_platWalkNodes[i]._deltaX = 1;
				_platWalkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_platX - gridIncr, _platY) && gridSub423CC1(_platX - gridIncr, _platY, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _platX - i;
				_platWalkNodes[i]._gridY1 = _platY;
				_platWalkNodes[i]._deltaX = -1;
				_platWalkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_platX, _platY + gridIncr) && gridSub423CC1(_platX, _platY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _platX;
				_platWalkNodes[i]._gridY1 = _platY + i;
				_platWalkNodes[i]._deltaX = 0;
				_platWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_platX, _platY - gridIncr) && gridSub423CC1(_platX, _platY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _platX;
				_platWalkNodes[i]._gridY1 = _platY - i;
				_platWalkNodes[i]._deltaX = 0;
				_platWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_platX + gridIncr, _platY + gridIncr) && gridSub423CC1(_platX + gridIncr, _platY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _platX + i;
				_platWalkNodes[i]._gridY1 = _platY + i;
				_platWalkNodes[i]._deltaX = 1;
				_platWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_platX - gridIncr, _platY + gridIncr) && gridSub423CC1(_platX - gridIncr, _platY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _platX - i;
				_platWalkNodes[i]._gridY1 = _platY + i;
				_platWalkNodes[i]._deltaX = -1;
				_platWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_platX + gridIncr, _platY - gridIncr) && gridSub423CC1(_platX + gridIncr, _platY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _platX + i;
				_platWalkNodes[i]._gridY1 = _platY - i;
				_platWalkNodes[i]._deltaX = 1;
				_platWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_platX - gridIncr, _platY - gridIncr) && gridSub423CC1(_platX - gridIncr, _platY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _platX - i;
				_platWalkNodes[i]._gridY1 = _platY - i;
				_platWalkNodes[i]._deltaX = -1;
				_platWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_platX + gridIncr, _platY) && gridSub42419A(_platX + gridIncr, _platY, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _platX + i;
				_platWalkNodes[i]._gridY1 = _platY;
				_platWalkNodes[i]._deltaX = 1;
				_platWalkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_platX - gridIncr, _platY) && gridSub42419A(_platX - gridIncr, _platY, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _platX - i;
				_platWalkNodes[i]._gridY1 = _platY;
				_platWalkNodes[i]._deltaX = -1;
				_platWalkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_platX, _platY + gridIncr) && gridSub42419A(_platX, _platY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _platX;
				_platWalkNodes[i]._gridY1 = _platY + i;
				_platWalkNodes[i]._deltaX = 0;
				_platWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_platX, _platY - gridIncr) && gridSub42419A(_platX, _platY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _platX;
				_platWalkNodes[i]._gridY1 = _platY - i;
				_platWalkNodes[i]._deltaX = 0;
				_platWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_platX + gridIncr, _platY + gridIncr) && gridSub42419A(_platX + gridIncr, _platY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _platX + i;
				_platWalkNodes[i]._gridY1 = _platY + i;
				_platWalkNodes[i]._deltaX = 1;
				_platWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_platX - gridIncr, _platY + gridIncr) && gridSub42419A(_platX - gridIncr, _platY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _platX - i;
				_platWalkNodes[i]._gridY1 = _platY + i;
				_platWalkNodes[i]._deltaX = -1;
				_platWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_platX + gridIncr, _platY - gridIncr) && gridSub42419A(_platX + gridIncr, _platY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _platX + i;
				_platWalkNodes[i]._gridY1 = _platY - i;
				_platWalkNodes[i]._deltaX = 1;
				_platWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_platX - gridIncr, _platY - gridIncr) && gridSub42419A(_platX - gridIncr, _platY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _platX - i;
				_platWalkNodes[i]._gridY1 = _platY - i;
				_platWalkNodes[i]._deltaX = -1;
				_platWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		++gridIncr;
	}

	return done;
}

bool GnapEngine::platypusWalkTo(int gridX, int gridY, int animationIndex, int sequenceId, int flags) {
	
	int datNum = flags & 3;
	bool done = false;

	_timers[1] = 60;

	if (gridX < 0)
		gridX = (_leftClickMouseX - _gridMinX + 37) / 75;

	if (gridY < 0)
		gridY = (_leftClickMouseY - _gridMinY + 24) / 48;

	_platWalkDestX = CLIP(gridX, 0, _gridMaxX - 1);
	_platWalkDestY = CLIP(gridY, 0, _gridMaxY - 1);

	if (animationIndex >= 0 && _platWalkDestX == _gnapX && _platWalkDestY == _gnapY)
		gnapWalkStep();

	if (gridSub423CC1(_platX, _platY, 0))
		done = true;

	if (!done && gridSub42419A(_platX, _platY, 0))
		done = true;
		
	if (!done && platFindPath3(_platX, _platY))
		done = true;

	if (!done)
		gridSub423750(_platX, _platY);

	int platSequenceId = _platypusSequenceId;
	int platId = _platypusId;
	int platSequenceDatNum = _platypusSequenceDatNum;

	for (int index = 0; index < _platWalkNodesCount; ++index) {
		_platWalkNodes[index]._id = index + 20 * _platWalkNodes[index]._gridY1;
		if (_platWalkNodes[index]._deltaX == 1 && _platWalkNodes[index]._deltaY == 0) {
			if (index % 2) {
				_gameSys->insertSequence(makeRid(datNum, 0x7CD), _platWalkNodes[index]._id,
					makeRid(platSequenceDatNum, platSequenceId), platId,
					kSeqScale | kSeqSyncWait, 0, 75 * _platWalkNodes[index]._gridX1 - _platGridX, 48 * _platWalkNodes[index]._gridY1 - _platGridY);
				_platWalkNodes[index]._sequenceId = 0x7CD;
				platSequenceId = 0x7CD;
			} else {
				_gameSys->insertSequence(makeRid(datNum, 0x7CE), _platWalkNodes[index]._id,
					makeRid(platSequenceDatNum, platSequenceId), platId,
					kSeqScale | kSeqSyncWait, 0, 75 * _platWalkNodes[index]._gridX1 - _platGridX, 48 * _platWalkNodes[index]._gridY1 - _platGridY);
				_platWalkNodes[index]._sequenceId = 0x7CE;
				platSequenceId = 0x7CE;
			}
		} else if (_platWalkNodes[index]._deltaX == -1 && _platWalkNodes[index]._deltaY == 0) {
			if (index % 2) {
				_gameSys->insertSequence(makeRid(datNum, 0x7CF), _platWalkNodes[index]._id,
					makeRid(platSequenceDatNum, platSequenceId), platId,
					kSeqScale | kSeqSyncWait, 0, 75 * _platWalkNodes[index]._gridX1 - _platGridX, 48 * _platWalkNodes[index]._gridY1 - _platGridY);
				_platWalkNodes[index]._sequenceId = 0x7CF;
				platSequenceId = 0x7CF;
			} else {
				_gameSys->insertSequence(makeRid(datNum, 0x7D0), _platWalkNodes[index]._id,
					makeRid(platSequenceDatNum, platSequenceId), platId,
					kSeqScale | kSeqSyncWait, 0, 75 * _platWalkNodes[index]._gridX1 - _platGridX, 48 * _platWalkNodes[index]._gridY1 - _platGridY);
				_platWalkNodes[index]._sequenceId = 0x7D0;
				platSequenceId = 0x7D0;
			}
		} else {
			if (_platWalkNodes[index]._deltaY == -1)
				_platWalkNodes[index]._id -= 10;
			else
				_platWalkNodes[index]._id += 10;
			int newSequenceId = getPlatypusWalkSequenceId(_platWalkNodes[index]._deltaX, _platWalkNodes[index]._deltaY);
			_gameSys->insertSequence(makeRid(datNum, newSequenceId), _platWalkNodes[index]._id,
				makeRid(platSequenceDatNum, platSequenceId), platId,
				kSeqScale | kSeqSyncWait, 0, 75 * _platWalkNodes[index]._gridX1 - _platGridX, 48 * _platWalkNodes[index]._gridY1 - _platGridY);
			_platWalkNodes[index]._sequenceId = newSequenceId;
			platSequenceId = newSequenceId;
		}
		platId = _platWalkNodes[index]._id;
		platSequenceDatNum = datNum;
	}

	if (flags & 8) {
		if (_platWalkNodesCount > 0) {
			_platypusSequenceId = platSequenceId;
			_platypusId = platId;
			_platypusSequenceDatNum = datNum;
			// CHECKME Not sure if this is correct...
			if (_platWalkNodes[_platWalkNodesCount - 1]._deltaX > 0)
				_platypusFacing = kDirNone;
			else if (_platWalkNodes[_platWalkNodesCount - 1]._deltaX < 0)
				_platypusFacing = kDirUnk4;
			else if (_platWalkNodes[_platWalkNodesCount - 1]._gridX1 % 2)
				_platypusFacing = kDirUnk4;
			else
				_platypusFacing = kDirNone;
			if (animationIndex >= 0)
				_gameSys->setAnimation(makeRid(_platypusSequenceDatNum, _platypusSequenceId), _platypusId, animationIndex);
		} else if (animationIndex >= 0) {
			_gameSys->setAnimation(0x107D3, 1, animationIndex);
			_gameSys->insertSequence(0x107D3, 1, 0, 0, kSeqNone, 0, 0, 0);
		}
	} else {
		if (sequenceId >= 0 && sequenceId != -1) {
			_platypusSequenceId = ridToEntryIndex(sequenceId);
			_platypusSequenceDatNum = ridToDatIndex(sequenceId);
			if (_platypusSequenceId == 0x7C2) {
				_platypusFacing = kDirNone;
			} else if (_platypusSequenceId == 0x7D2) {
				_platypusFacing = kDirUnk4;
			}
		} else {
			if (_platWalkNodesCount > 0) {
				if (_platWalkNodes[_platWalkNodesCount - 1]._deltaX > 0) {
					_platypusSequenceId = 0x7C2;
					_platypusFacing = kDirNone;
				} else if (_platWalkNodes[_platWalkNodesCount - 1]._deltaX < 0) {
					_platypusSequenceId = 0x7D2;
					_platypusFacing = kDirUnk4;
				} else if (_platWalkNodes[0]._deltaX > 0) {
					_platypusSequenceId = 0x7C2;
					_platypusFacing = kDirNone;
				} else if (_platWalkNodes[0]._deltaX < 0) {
					_platypusSequenceId = 0x7D2;
					_platypusFacing = kDirUnk4;
				} else {
					_platypusSequenceId = 0x7D2;
					_platypusFacing = kDirUnk4;
				}
			} else if (_platypusFacing != kDirNone) {
				_platypusSequenceId = 0x7D2;
			} else {
				_platypusSequenceId = 0x7C2;
			}
			_platypusSequenceDatNum = datNum;
		}

		if (animationIndex < 0) {
			_platypusId = 20 * _platWalkDestY;
		} else {
			_platypusId = animationIndex + 20 * _platWalkDestY;
			_gameSys->setAnimation(makeRid(_platypusSequenceDatNum, _platypusSequenceId), animationIndex + 20 * _platWalkDestY, animationIndex);
		}

		if (flags & 4)
			_gameSys->insertSequence(makeRid(_platypusSequenceDatNum, _platypusSequenceId), _platypusId,
				makeRid(platSequenceDatNum, platSequenceId), platId,
				9, 0, 0, 0);
		else
			_gameSys->insertSequence(makeRid(_platypusSequenceDatNum, _platypusSequenceId), _platypusId,
				makeRid(platSequenceDatNum, platSequenceId), platId,
				9, 0, 75 * _platWalkDestX - _platGridX, 48 * _platWalkDestY - _platGridY);
	}

	_platX = _platWalkDestX;
	_platY = _platWalkDestY;
	
	return done;
}

void GnapEngine::platypusWalkStep() {
	bool done = false;
	for (int i = 1; !done && i < _gridMaxX; ++i) {
		done = true;
		if (!isPointBlocked(_platX + i, _platY))
			platypusWalkTo(_platX + i, _platY, -1, -1, 1);
		else if (!isPointBlocked(_platX - i, _platY))
			platypusWalkTo(_platX - i, _platY, -1, -1, 1);
		else if (!isPointBlocked(_platX, _platY + 1))
			platypusWalkTo(_platX, _platY + 1, -1, -1, 1);
		else if (!isPointBlocked(_platX, _platY - 1))
			platypusWalkTo(_platX, _platY - 1, -1, -1, 1);
		else if (!isPointBlocked(_platX + 1, _platY + 1))
			platypusWalkTo(_platX + 1, _platY + 1, -1, -1, 1);
		else if (!isPointBlocked(_platX - 1, _platY + 1))
			platypusWalkTo(_platX - 1, _platY + 1, -1, -1, 1);
		else if (!isPointBlocked(_platX + 1, _platY - 1))
			platypusWalkTo(_platX + 1, _platY - 1, -1, -1, 1);
		else if (!isPointBlocked(_platX - 1, _platY - 1))
			platypusWalkTo(_platX - 1, _platY - 1, -1, -1, 1);
		else
			done = false;
	}
}

void GnapEngine::platypusMakeRoom() {
	int rndGridX, rndGridY;
	do {
		rndGridY = getRandom(_gridMaxY);
		rndGridX = getRandom(_gridMaxX);
	} while (ABS(rndGridX - _platX) > 4 || ABS(rndGridY - _platY) > 3 ||
		isPointBlocked(rndGridX, rndGridY));
	platypusWalkTo(rndGridX, rndGridY, -1, -1, 1);
}

} // End of namespace Gnap
