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
	_gnap->_gridX = 410 - gridMinX;
	_gnap->_gridY = 450 - gridMinY;
	_plat->_gridX = 396 - gridMinX;
	_plat->_gridY = 347 - gridMinY;
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

bool GnapEngine::isPointBlocked(Common::Point gridPos) {
	return isPointBlocked(gridPos.x, gridPos.y);
}

bool GnapEngine::isPointBlocked(int gridX, int gridY) {

	if (gridX < 0 || gridX >= _gridMaxX || gridY < 0 || gridY >= _gridMaxY)
		return true;

	if ((_gnap->_pos == Common::Point(gridX, gridY)) || (gridX == _plat->_pos.x && gridY == _plat->_pos.y))
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

bool GnapEngine::gnapFindPath4(int gridX, int gridY) {
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

bool GnapEngine::gnapFindPath1(int gridX, int gridY, int index) {
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

bool GnapEngine::gnapFindPath2(int gridX, int gridY, int index) {
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
		if (!isPointBlocked(gridX + gridIncr, gridY) && gnapFindPath1(gridX + gridIncr, gridY, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX + i;
				_gnapWalkNodes[i]._gridY1 = gridY;
				_gnapWalkNodes[i]._deltaX = 1;
				_gnapWalkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX - gridIncr, gridY) && gnapFindPath1(gridX - gridIncr, gridY, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX - i;
				_gnapWalkNodes[i]._gridY1 = gridY;
				_gnapWalkNodes[i]._deltaX = -1;
				_gnapWalkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX, gridY + gridIncr) && gnapFindPath1(gridX, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX;
				_gnapWalkNodes[i]._gridY1 = gridY + i;
				_gnapWalkNodes[i]._deltaX = 0;
				_gnapWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX, gridY - gridIncr) && gnapFindPath1(gridX, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX;
				_gnapWalkNodes[i]._gridY1 = gridY - i;
				_gnapWalkNodes[i]._deltaX = 0;
				_gnapWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX + gridIncr, gridY + gridIncr) && gnapFindPath1(gridX + gridIncr, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX + i;
				_gnapWalkNodes[i]._gridY1 = gridY + i;
				_gnapWalkNodes[i]._deltaX = 1;
				_gnapWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX - gridIncr, gridY + gridIncr) && gnapFindPath1(gridX - gridIncr, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX - i;
				_gnapWalkNodes[i]._gridY1 = gridY + i;
				_gnapWalkNodes[i]._deltaX = -1;
				_gnapWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX + gridIncr, gridY - gridIncr) && gnapFindPath1(gridX + gridIncr, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX + i;
				_gnapWalkNodes[i]._gridY1 = gridY - i;
				_gnapWalkNodes[i]._deltaX = 1;
				_gnapWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX - gridIncr, gridY - gridIncr) && gnapFindPath1(gridX - gridIncr, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX - i;
				_gnapWalkNodes[i]._gridY1 = gridY - i;
				_gnapWalkNodes[i]._deltaX = -1;
				_gnapWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX + gridIncr, gridY) && gnapFindPath2(gridX + gridIncr, gridY, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX + i;
				_gnapWalkNodes[i]._gridY1 = gridY;
				_gnapWalkNodes[i]._deltaX = 1;
				_gnapWalkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX - gridIncr, gridY) && gnapFindPath2(gridX - gridIncr, gridY, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX - i;
				_gnapWalkNodes[i]._gridY1 = gridY;
				_gnapWalkNodes[i]._deltaX = -1;
				_gnapWalkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX, gridY + gridIncr) && gnapFindPath2(gridX, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX;
				_gnapWalkNodes[i]._gridY1 = gridY + i;
				_gnapWalkNodes[i]._deltaX = 0;
				_gnapWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX, gridY - gridIncr) && gnapFindPath2(gridX, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX;
				_gnapWalkNodes[i]._gridY1 = gridY - i;
				_gnapWalkNodes[i]._deltaX = 0;
				_gnapWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX + gridIncr, gridY + gridIncr) && gnapFindPath2(gridX + gridIncr, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX + i;
				_gnapWalkNodes[i]._gridY1 = gridY + i;
				_gnapWalkNodes[i]._deltaX = 1;
				_gnapWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX - gridIncr, gridY + gridIncr) && gnapFindPath2(gridX - gridIncr, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX - i;
				_gnapWalkNodes[i]._gridY1 = gridY + i;
				_gnapWalkNodes[i]._deltaX = -1;
				_gnapWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX + gridIncr, gridY - gridIncr) && gnapFindPath2(gridX + gridIncr, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_gnapWalkNodes[i]._gridX1 = gridX + i;
				_gnapWalkNodes[i]._gridY1 = gridY - i;
				_gnapWalkNodes[i]._deltaX = 1;
				_gnapWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(gridX - gridIncr, gridY - gridIncr) && gnapFindPath2(gridX - gridIncr, gridY - gridIncr, gridIncr)) {
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

bool GnapEngine::gnapWalkTo(Common::Point gridPos, int animationIndex, int sequenceId, int flags) {
	int datNum = flags & 3;
	bool done = false;

	_timers[2] = 200;
	_timers[3] = 300;

	int gridX = gridPos.x;
	if (gridX < 0)
		gridX = (_leftClickMouseX - _gridMinX + 37) / 75;

	int gridY = gridPos.y;
	if (gridY < 0)
		gridY = (_leftClickMouseY - _gridMinY + 24) / 48;

	_gnapWalkDestX = CLIP(gridX, 0, _gridMaxX - 1);
	_gnapWalkDestY = CLIP(gridY, 0, _gridMaxY - 1);

	if (animationIndex >= 0 && _gnapWalkDestX == _plat->_pos.x && _gnapWalkDestY == _plat->_pos.y)
		platypusMakeRoom();

	// TODO: Simplify the cascade of Ifs
	if (gnapFindPath1(_gnap->_pos.x, _gnap->_pos.y, 0))
		done = true;

	if (!done && gnapFindPath2(_gnap->_pos.x, _gnap->_pos.y, 0))
		done = true;

	if (!done && gnapFindPath3(_gnap->_pos.x, _gnap->_pos.y))
		done = true;

	if (!done && gnapFindPath4(_gnap->_pos.x, _gnap->_pos.y))
		done = true;

	gnapIdle();

	int gnapSequenceId = _gnap->_sequenceId;
	int gnapId = _gnap->_id;
	int gnapSequenceDatNum = _gnap->_sequenceDatNum;

	debugC(kDebugBasic, "_gnapWalkNodesCount: %d", _gnapWalkNodesCount);

	for (int index = 0; index < _gnapWalkNodesCount; ++index) {
		_gnapWalkNodes[index]._id = index + 20 * _gnapWalkNodes[index]._gridY1;
		if (_gnapWalkNodes[index]._deltaX == 1 && _gnapWalkNodes[index]._deltaY == 0) {
			if (index % 2) {
				_gameSys->insertSequence(makeRid(datNum, 0x7AB), _gnapWalkNodes[index]._id,
					makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
					kSeqScale | kSeqSyncWait, 0, 75 * _gnapWalkNodes[index]._gridX1 - _gnap->_gridX, 48 * _gnapWalkNodes[index]._gridY1 - _gnap->_gridY);
				_gnapWalkNodes[index]._sequenceId = 0x7AB;
				gnapSequenceId = 0x7AB;
			} else {
				_gameSys->insertSequence(makeRid(datNum, 0x7AC), _gnapWalkNodes[index]._id,
					makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
					kSeqScale | kSeqSyncWait, 0, 75 * _gnapWalkNodes[index]._gridX1 - _gnap->_gridX, 48 * _gnapWalkNodes[index]._gridY1 - _gnap->_gridY);
				_gnapWalkNodes[index]._sequenceId = 0x7AC;
				gnapSequenceId = 0x7AC;
			}
		} else if (_gnapWalkNodes[index]._deltaX == -1 && _gnapWalkNodes[index]._deltaY == 0) {
			if (index % 2) {
				_gameSys->insertSequence(makeRid(datNum, 0x7AF), _gnapWalkNodes[index]._id,
					makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
					kSeqScale | kSeqSyncWait, 0, 75 * _gnapWalkNodes[index]._gridX1 - _gnap->_gridX, 48 * _gnapWalkNodes[index]._gridY1 - _gnap->_gridY);
				_gnapWalkNodes[index]._sequenceId = 0x7AF;
				gnapSequenceId = 0x7AF;
			} else {
				_gameSys->insertSequence(makeRid(datNum, 0x7B0), _gnapWalkNodes[index]._id,
					makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
					kSeqScale | kSeqSyncWait, 0, 75 * _gnapWalkNodes[index]._gridX1 - _gnap->_gridX, 48 * _gnapWalkNodes[index]._gridY1 - _gnap->_gridY);
				_gnapWalkNodes[index]._sequenceId = 0x7B0;
				gnapSequenceId = 0x7B0;
			}
		} else {
			if (_gnapWalkNodes[index]._deltaY == -1)
				_gnapWalkNodes[index]._id -= 10;
			else
				_gnapWalkNodes[index]._id += 10;
			int newSequenceId = _gnap->getWalkSequenceId(_gnapWalkNodes[index]._deltaX, _gnapWalkNodes[index]._deltaY);
			_gameSys->insertSequence(makeRid(datNum, newSequenceId), _gnapWalkNodes[index]._id,
				makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
				kSeqScale | kSeqSyncWait, 0, 75 * _gnapWalkNodes[index]._gridX1 - _gnap->_gridX, 48 * _gnapWalkNodes[index]._gridY1 - _gnap->_gridY);
			_gnapWalkNodes[index]._sequenceId = newSequenceId;
			gnapSequenceId = newSequenceId;
		}
		gnapId = _gnapWalkNodes[index]._id;
		gnapSequenceDatNum = datNum;
	}

	if (flags & 8) {
		if (_gnapWalkNodesCount > 0) {
			_gnap->_sequenceId = gnapSequenceId;
			_gnap->_id = gnapId;
			_gnap->_idleFacing = getGnapWalkFacing(_gnapWalkNodes[_gnapWalkNodesCount - 1]._deltaX, _gnapWalkNodes[_gnapWalkNodesCount - 1]._deltaY);
			_gnap->_sequenceDatNum = datNum;
			if (animationIndex >= 0)
				_gameSys->setAnimation(makeRid(_gnap->_sequenceDatNum, _gnap->_sequenceId), _gnap->_id, animationIndex);
		} else if (animationIndex >= 0) {
			_gameSys->setAnimation(0x107D3, 1, animationIndex);
			_gameSys->insertSequence(0x107D3, 1, 0, 0, kSeqNone, 0, 0, 0);
		}
	} else {
		if (sequenceId >= 0 && sequenceId != -1) {
			_gnap->_sequenceId = ridToEntryIndex(sequenceId);
			_gnap->_sequenceDatNum = ridToDatIndex(sequenceId);
			if (_gnap->_sequenceId == 0x7B9) {
				_gnap->_idleFacing = kDirBottomRight;
			} else {
				switch (_gnap->_sequenceId) {
				case 0x7BA:
					_gnap->_idleFacing = kDirBottomLeft;
					break;
				case 0x7BB:
					_gnap->_idleFacing = kDirUpRight;
					break;
				case 0x7BC:
					_gnap->_idleFacing = kDirUpLeft;
					break;
				}
			}
		} else {
			if (_gnapWalkNodesCount > 0) {
				_gnap->_sequenceId = getGnapWalkStopSequenceId(_gnapWalkNodes[_gnapWalkNodesCount - 1]._deltaX, _gnapWalkNodes[_gnapWalkNodesCount - 1]._deltaY);
				_gnap->_idleFacing = getGnapWalkFacing(_gnapWalkNodes[_gnapWalkNodesCount - 1]._deltaX, _gnapWalkNodes[_gnapWalkNodesCount - 1]._deltaY);
			} else if (gridX >= 0 || gridY >= 0) {
				switch (_gnap->_idleFacing) {
				case kDirBottomRight:
					_gnap->_sequenceId = 0x7B9;
					break;
				case kDirBottomLeft:
					_gnap->_sequenceId = 0x7BA;
					break;
				case kDirUpRight:
					_gnap->_sequenceId = 0x7BB;
					break;
				default:
					_gnap->_sequenceId = 0x7BC;
					break;
				}
			} else {
				//TODO: simplify the checks by using v10 and v11
				int v10 = _leftClickMouseX - (_gridMinX + 75 * _gnap->_pos.x);
				int v11 = _leftClickMouseY - (_gridMinY + 48 * _gnap->_pos.y);
				if (_leftClickMouseX == _gridMinX + 75 * _gnap->_pos.x)
					++v10;
				if (_leftClickMouseY == _gridMinY + 48 * _gnap->_pos.y)
					v11 = 1;
				_gnap->_sequenceId = getGnapWalkStopSequenceId(v10 / abs(v10), v11 / abs(v11));
				_gnap->_idleFacing = getGnapWalkFacing(v10 / abs(v10), v11 / abs(v11));
			}
			_gnap->_sequenceDatNum = datNum;
		}

		if (animationIndex < 0) {
			_gnap->_id = 20 * _gnapWalkDestY + 1;
		} else {
			_gnap->_id = _gnapWalkNodesCount + animationIndex + 20 * _gnapWalkDestY;
			_gameSys->setAnimation(makeRid(_gnap->_sequenceDatNum, _gnap->_sequenceId), _gnapWalkNodesCount + animationIndex + 20 * _gnapWalkDestY, animationIndex);
		}

		if (flags & 4) {
			_gameSys->insertSequence(makeRid(_gnap->_sequenceDatNum, _gnap->_sequenceId), _gnap->_id,
				makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
				kSeqScale | kSeqSyncWait, 0, 0, 0);
		} else {
			_gameSys->insertSequence(makeRid(_gnap->_sequenceDatNum, _gnap->_sequenceId), _gnap->_id,
				makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
				kSeqScale | kSeqSyncWait, 0, 75 * _gnapWalkDestX - _gnap->_gridX, 48 * _gnapWalkDestY - _gnap->_gridY);
		}
	}

	_gnap->_pos = Common::Point(_gnapWalkDestX, _gnapWalkDestY);

	return done;
}

void GnapEngine::gnapWalkStep() {
	for (int i = 1; i < _gridMaxX; ++i) {
		Common::Point checkPt = Common::Point(_gnap->_pos.x + i, _gnap->_pos.y);
		if (!isPointBlocked(checkPt)) {
			gnapWalkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_gnap->_pos.x - i, _gnap->_pos.y);
		if (!isPointBlocked(checkPt)) {
			gnapWalkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_gnap->_pos.x, _gnap->_pos.y + 1);
		if (!isPointBlocked(checkPt)) {
			gnapWalkTo(checkPt, -1, -1, 1);
			break;
		}
		
		checkPt = Common::Point(_gnap->_pos.x, _gnap->_pos.y - 1);
		if (!isPointBlocked(checkPt)) {
			gnapWalkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_gnap->_pos.x + 1, _gnap->_pos.y + 1);
		if (!isPointBlocked(checkPt)) {
			gnapWalkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_gnap->_pos.x - 1, _gnap->_pos.y + 1);
		if (!isPointBlocked(checkPt)) {
			gnapWalkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_gnap->_pos.x + 1, _gnap->_pos.y - 1);
		if (!isPointBlocked(checkPt)) {
			gnapWalkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_gnap->_pos.x - 1, _gnap->_pos.y - 1);
		if (!isPointBlocked(checkPt)) {
			gnapWalkTo(checkPt, -1, -1, 1);
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

bool GnapEngine::platFindPath4(int gridX, int gridY) {
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

bool GnapEngine::platFindPath1(int gridX, int gridY, int index) {
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

bool GnapEngine::platFindPath2(int gridX, int gridY, int index) {
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
		if (!isPointBlocked(_plat->_pos.x + gridIncr, _plat->_pos.y) && platFindPath1(_plat->_pos.x + gridIncr, _plat->_pos.y, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _plat->_pos.x + i;
				_platWalkNodes[i]._gridY1 = _plat->_pos.y;
				_platWalkNodes[i]._deltaX = 1;
				_platWalkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_plat->_pos.x - gridIncr, _plat->_pos.y) && platFindPath1(_plat->_pos.x - gridIncr, _plat->_pos.y, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _plat->_pos.x - i;
				_platWalkNodes[i]._gridY1 = _plat->_pos.y;
				_platWalkNodes[i]._deltaX = -1;
				_platWalkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_plat->_pos.x, _plat->_pos.y + gridIncr) && platFindPath1(_plat->_pos.x, _plat->_pos.y + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _plat->_pos.x;
				_platWalkNodes[i]._gridY1 = _plat->_pos.y + i;
				_platWalkNodes[i]._deltaX = 0;
				_platWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_plat->_pos.x, _plat->_pos.y - gridIncr) && platFindPath1(_plat->_pos.x, _plat->_pos.y - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _plat->_pos.x;
				_platWalkNodes[i]._gridY1 = _plat->_pos.y - i;
				_platWalkNodes[i]._deltaX = 0;
				_platWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_plat->_pos.x + gridIncr, _plat->_pos.y + gridIncr) && platFindPath1(_plat->_pos.x + gridIncr, _plat->_pos.y + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _plat->_pos.x + i;
				_platWalkNodes[i]._gridY1 = _plat->_pos.y + i;
				_platWalkNodes[i]._deltaX = 1;
				_platWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_plat->_pos.x - gridIncr, _plat->_pos.y + gridIncr) && platFindPath1(_plat->_pos.x - gridIncr, _plat->_pos.y + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _plat->_pos.x - i;
				_platWalkNodes[i]._gridY1 = _plat->_pos.y + i;
				_platWalkNodes[i]._deltaX = -1;
				_platWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_plat->_pos.x + gridIncr, _plat->_pos.y - gridIncr) && platFindPath1(_plat->_pos.x + gridIncr, _plat->_pos.y - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _plat->_pos.x + i;
				_platWalkNodes[i]._gridY1 = _plat->_pos.y - i;
				_platWalkNodes[i]._deltaX = 1;
				_platWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_plat->_pos.x - gridIncr, _plat->_pos.y - gridIncr) && platFindPath1(_plat->_pos.x - gridIncr, _plat->_pos.y - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _plat->_pos.x - i;
				_platWalkNodes[i]._gridY1 = _plat->_pos.y - i;
				_platWalkNodes[i]._deltaX = -1;
				_platWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_plat->_pos.x + gridIncr, _plat->_pos.y) && platFindPath2(_plat->_pos.x + gridIncr, _plat->_pos.y, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _plat->_pos.x + i;
				_platWalkNodes[i]._gridY1 = _plat->_pos.y;
				_platWalkNodes[i]._deltaX = 1;
				_platWalkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_plat->_pos.x - gridIncr, _plat->_pos.y) && platFindPath2(_plat->_pos.x - gridIncr, _plat->_pos.y, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _plat->_pos.x - i;
				_platWalkNodes[i]._gridY1 = _plat->_pos.y;
				_platWalkNodes[i]._deltaX = -1;
				_platWalkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_plat->_pos.x, _plat->_pos.y + gridIncr) && platFindPath2(_plat->_pos.x, _plat->_pos.y + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _plat->_pos.x;
				_platWalkNodes[i]._gridY1 = _plat->_pos.y + i;
				_platWalkNodes[i]._deltaX = 0;
				_platWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_plat->_pos.x, _plat->_pos.y - gridIncr) && platFindPath2(_plat->_pos.x, _plat->_pos.y - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _plat->_pos.x;
				_platWalkNodes[i]._gridY1 = _plat->_pos.y - i;
				_platWalkNodes[i]._deltaX = 0;
				_platWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_plat->_pos.x + gridIncr, _plat->_pos.y + gridIncr) && platFindPath2(_plat->_pos.x + gridIncr, _plat->_pos.y + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _plat->_pos.x + i;
				_platWalkNodes[i]._gridY1 = _plat->_pos.y + i;
				_platWalkNodes[i]._deltaX = 1;
				_platWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_plat->_pos.x - gridIncr, _plat->_pos.y + gridIncr) && platFindPath2(_plat->_pos.x - gridIncr, _plat->_pos.y + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _plat->_pos.x - i;
				_platWalkNodes[i]._gridY1 = _plat->_pos.y + i;
				_platWalkNodes[i]._deltaX = -1;
				_platWalkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_plat->_pos.x + gridIncr, _plat->_pos.y - gridIncr) && platFindPath2(_plat->_pos.x + gridIncr, _plat->_pos.y - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _plat->_pos.x + i;
				_platWalkNodes[i]._gridY1 = _plat->_pos.y - i;
				_platWalkNodes[i]._deltaX = 1;
				_platWalkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!isPointBlocked(_plat->_pos.x - gridIncr, _plat->_pos.y - gridIncr) && platFindPath2(_plat->_pos.x - gridIncr, _plat->_pos.y - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_platWalkNodes[i]._gridX1 = _plat->_pos.x - i;
				_platWalkNodes[i]._gridY1 = _plat->_pos.y - i;
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

bool GnapEngine::platypusWalkTo(Common::Point gridPos, int animationIndex, int sequenceId, int flags) {
	int datNum = flags & 3;
	bool done = false;

	_timers[1] = 60;

	int gridX = gridPos.x;
	if (gridX < 0)
		gridX = (_leftClickMouseX - _gridMinX + 37) / 75;

	int gridY = gridPos.y;
	if (gridY < 0)
		gridY = (_leftClickMouseY - _gridMinY + 24) / 48;

	_platWalkDestX = CLIP(gridX, 0, _gridMaxX - 1);
	_platWalkDestY = CLIP(gridY, 0, _gridMaxY - 1);

	if (animationIndex >= 0 && _gnap->_pos == Common::Point(_platWalkDestX, _platWalkDestY))
		gnapWalkStep();

	if (platFindPath1(_plat->_pos.x, _plat->_pos.y, 0))
		done = true;

	if (!done && platFindPath2(_plat->_pos.x, _plat->_pos.y, 0))
		done = true;

	if (!done && platFindPath3(_plat->_pos.x, _plat->_pos.y))
		done = true;

	if (!done && platFindPath4(_plat->_pos.x, _plat->_pos.y))
		done = true;

	int platSequenceId = _plat->_sequenceId;
	int platId = _plat->_id;
	int platSequenceDatNum = _plat->_sequenceDatNum;

	for (int index = 0; index < _platWalkNodesCount; ++index) {
		_platWalkNodes[index]._id = index + 20 * _platWalkNodes[index]._gridY1;
		if (_platWalkNodes[index]._deltaX == 1 && _platWalkNodes[index]._deltaY == 0) {
			if (index % 2) {
				_gameSys->insertSequence(makeRid(datNum, 0x7CD), _platWalkNodes[index]._id,
					makeRid(platSequenceDatNum, platSequenceId), platId,
					kSeqScale | kSeqSyncWait, 0, 75 * _platWalkNodes[index]._gridX1 - _plat->_gridX, 48 * _platWalkNodes[index]._gridY1 - _plat->_gridY);
				_platWalkNodes[index]._sequenceId = 0x7CD;
				platSequenceId = 0x7CD;
			} else {
				_gameSys->insertSequence(makeRid(datNum, 0x7CE), _platWalkNodes[index]._id,
					makeRid(platSequenceDatNum, platSequenceId), platId,
					kSeqScale | kSeqSyncWait, 0, 75 * _platWalkNodes[index]._gridX1 - _plat->_gridX, 48 * _platWalkNodes[index]._gridY1 - _plat->_gridY);
				_platWalkNodes[index]._sequenceId = 0x7CE;
				platSequenceId = 0x7CE;
			}
		} else if (_platWalkNodes[index]._deltaX == -1 && _platWalkNodes[index]._deltaY == 0) {
			if (index % 2) {
				_gameSys->insertSequence(makeRid(datNum, 0x7CF), _platWalkNodes[index]._id,
					makeRid(platSequenceDatNum, platSequenceId), platId,
					kSeqScale | kSeqSyncWait, 0, 75 * _platWalkNodes[index]._gridX1 - _plat->_gridX, 48 * _platWalkNodes[index]._gridY1 - _plat->_gridY);
				_platWalkNodes[index]._sequenceId = 0x7CF;
				platSequenceId = 0x7CF;
			} else {
				_gameSys->insertSequence(makeRid(datNum, 0x7D0), _platWalkNodes[index]._id,
					makeRid(platSequenceDatNum, platSequenceId), platId,
					kSeqScale | kSeqSyncWait, 0, 75 * _platWalkNodes[index]._gridX1 - _plat->_gridX, 48 * _platWalkNodes[index]._gridY1 - _plat->_gridY);
				_platWalkNodes[index]._sequenceId = 0x7D0;
				platSequenceId = 0x7D0;
			}
		} else {
			if (_platWalkNodes[index]._deltaY == -1)
				_platWalkNodes[index]._id -= 10;
			else
				_platWalkNodes[index]._id += 10;
			int newSequenceId = _plat->getWalkSequenceId(_platWalkNodes[index]._deltaX, _platWalkNodes[index]._deltaY);
			_gameSys->insertSequence(makeRid(datNum, newSequenceId), _platWalkNodes[index]._id,
				makeRid(platSequenceDatNum, platSequenceId), platId,
				kSeqScale | kSeqSyncWait, 0, 75 * _platWalkNodes[index]._gridX1 - _plat->_gridX, 48 * _platWalkNodes[index]._gridY1 - _plat->_gridY);
			_platWalkNodes[index]._sequenceId = newSequenceId;
			platSequenceId = newSequenceId;
		}
		platId = _platWalkNodes[index]._id;
		platSequenceDatNum = datNum;
	}

	if (flags & 8) {
		if (_platWalkNodesCount > 0) {
			_plat->_sequenceId = platSequenceId;
			_plat->_id = platId;
			_plat->_sequenceDatNum = datNum;
			// CHECKME Not sure if this is correct...
			if (_platWalkNodes[_platWalkNodesCount - 1]._deltaX > 0)
				_plat->_idleFacing = kDirNone;
			else if (_platWalkNodes[_platWalkNodesCount - 1]._deltaX < 0)
				_plat->_idleFacing = kDirUnk4;
			else if (_platWalkNodes[_platWalkNodesCount - 1]._gridX1 % 2)
				_plat->_idleFacing = kDirUnk4;
			else
				_plat->_idleFacing = kDirNone;
			if (animationIndex >= 0)
				_gameSys->setAnimation(makeRid(_plat->_sequenceDatNum, _plat->_sequenceId), _plat->_id, animationIndex);
		} else if (animationIndex >= 0) {
			_gameSys->setAnimation(0x107D3, 1, animationIndex);
			_gameSys->insertSequence(0x107D3, 1, 0, 0, kSeqNone, 0, 0, 0);
		}
	} else {
		if (sequenceId >= 0 && sequenceId != -1) {
			_plat->_sequenceId = ridToEntryIndex(sequenceId);
			_plat->_sequenceDatNum = ridToDatIndex(sequenceId);
			if (_plat->_sequenceId == 0x7C2) {
				_plat->_idleFacing = kDirNone;
			} else if (_plat->_sequenceId == 0x7D2) {
				_plat->_idleFacing = kDirUnk4;
			}
		} else {
			if (_platWalkNodesCount > 0) {
				if (_platWalkNodes[_platWalkNodesCount - 1]._deltaX > 0) {
					_plat->_sequenceId = 0x7C2;
					_plat->_idleFacing = kDirNone;
				} else if (_platWalkNodes[_platWalkNodesCount - 1]._deltaX < 0) {
					_plat->_sequenceId = 0x7D2;
					_plat->_idleFacing = kDirUnk4;
				} else if (_platWalkNodes[0]._deltaX > 0) {
					_plat->_sequenceId = 0x7C2;
					_plat->_idleFacing = kDirNone;
				} else if (_platWalkNodes[0]._deltaX < 0) {
					_plat->_sequenceId = 0x7D2;
					_plat->_idleFacing = kDirUnk4;
				} else {
					_plat->_sequenceId = 0x7D2;
					_plat->_idleFacing = kDirUnk4;
				}
			} else if (_plat->_idleFacing != kDirNone) {
				_plat->_sequenceId = 0x7D2;
			} else {
				_plat->_sequenceId = 0x7C2;
			}
			_plat->_sequenceDatNum = datNum;
		}

		if (animationIndex < 0) {
			_plat->_id = 20 * _platWalkDestY;
		} else {
			_plat->_id = animationIndex + 20 * _platWalkDestY;
			_gameSys->setAnimation(makeRid(_plat->_sequenceDatNum, _plat->_sequenceId), animationIndex + 20 * _platWalkDestY, animationIndex);
		}

		if (flags & 4)
			_gameSys->insertSequence(makeRid(_plat->_sequenceDatNum, _plat->_sequenceId), _plat->_id,
				makeRid(platSequenceDatNum, platSequenceId), platId,
				9, 0, 0, 0);
		else
			_gameSys->insertSequence(makeRid(_plat->_sequenceDatNum, _plat->_sequenceId), _plat->_id,
				makeRid(platSequenceDatNum, platSequenceId), platId,
				9, 0, 75 * _platWalkDestX - _plat->_gridX, 48 * _platWalkDestY - _plat->_gridY);
	}

	_plat->_pos = Common::Point(_platWalkDestX, _platWalkDestY);

	return done;
}

void GnapEngine::platypusWalkStep() {
	for (int i = 1; i < _gridMaxX; ++i) {
		Common::Point checkPt = Common::Point(_plat->_pos.x + i, _plat->_pos.y);
		if (!isPointBlocked(checkPt)) {
			platypusWalkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_plat->_pos.x - i, _plat->_pos.y);
		if (!isPointBlocked(checkPt)) {
			platypusWalkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_plat->_pos.x, _plat->_pos.y + 1);
		if (!isPointBlocked(checkPt)) {
			platypusWalkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_plat->_pos.x, _plat->_pos.y - 1);
		if (!isPointBlocked(checkPt)) {
			platypusWalkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_plat->_pos.x + 1, _plat->_pos.y + 1);
		if (!isPointBlocked(checkPt)) {
			platypusWalkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_plat->_pos.x - 1, _plat->_pos.y + 1);
		if (!isPointBlocked(checkPt)) {
			platypusWalkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_plat->_pos.x + 1, _plat->_pos.y - 1);
		if (!isPointBlocked(checkPt)) {
			platypusWalkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_plat->_pos.x - 1, _plat->_pos.y - 1);
		if (!isPointBlocked(checkPt)) {
			platypusWalkTo(checkPt, -1, -1, 1);
			break;
		}
	}
}

void GnapEngine::platypusMakeRoom() {
	int rndGridX, rndGridY;
	do {
		rndGridY = getRandom(_gridMaxY);
		rndGridX = getRandom(_gridMaxX);
	} while (ABS(rndGridX - _plat->_pos.x) > 4 || ABS(rndGridY - _plat->_pos.y) > 3 ||
		isPointBlocked(rndGridX, rndGridY));
	platypusWalkTo(Common::Point(rndGridX, rndGridY), -1, -1, 1);
}

} // End of namespace Gnap
