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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

bool GnapEngine::isPointBlocked(Common::Point gridPos) {
	return isPointBlocked(gridPos.x, gridPos.y);
}

bool GnapEngine::isPointBlocked(int gridX, int gridY) {

	if (gridX < 0 || gridX >= _gridMaxX || gridY < 0 || gridY >= _gridMaxY)
		return true;

	if ((_gnap->_pos == Common::Point(gridX, gridY)) || (_plat->_pos == Common::Point(gridX, gridY)))
		return true;

	Common::Point pos = Common::Point(_gridMinX + 75 * gridX, _gridMinY + 48 * gridY);

	for (int i = 0; i < _hotspotsCount; ++i) {
		if (_hotspots[i].isPointInside(pos) && !(_hotspots[i]._flags & SF_WALKABLE))
			return true;
	}

	return false;
}

/******************************************************************************/

int PlayerGnap::getWalkStopSequenceId(int deltaX, int deltaY) {
	static const int gnapWalkStopSequenceIds[9] = {
		0x7BC, 0x7BA, 0x7BA,
		0x7BC, 0x000, 0x7BA,
		0x7BB, 0x7B9, 0x7B9
	};

	int id = 3 * (deltaX + 1) + deltaY + 1;
	assert (id >= 0 && id < 9 );
	return gnapWalkStopSequenceIds[id];
}

Facing PlayerGnap::getWalkFacing(int deltaX, int deltaY) {
	static const Facing gnapWalkFacings[9] = {
		kDirUpLeft, kDirBottomLeft, kDirBottomLeft,
		kDirUpLeft, kDirIdleLeft, kDirBottomLeft,
		kDirUpRight, kDirBottomRight, kDirBottomRight
	};

	int id = 3 * (deltaX + 1) + deltaY + 1;
	assert (id >= 0 && id < 9 );
	return gnapWalkFacings[id];
}

bool PlayerGnap::findPath1(int gridX, int gridY, int index) {
	_walkNodesCount = index;
	_walkDirXIncr = 0;
	_walkDirYIncr = 0;
	_walkDeltaX = ABS(_walkDestX - gridX);
	_walkDeltaY = ABS(_walkDestY - gridY);

	if (_walkDeltaX)
		_walkDirX = (_walkDestX - gridX) / _walkDeltaX;
	else
		_walkDirX = 0;

	if (_walkDeltaY)
		_walkDirY = (_walkDestY - gridY) / _walkDeltaY;
	else
		_walkDirY = 0;

	while (_walkDirXIncr < _walkDeltaX && _walkDirYIncr < _walkDeltaY) {
		_walkNodes[_walkNodesCount]._gridX1 = gridX + _walkDirX * _walkDirXIncr;
		_walkNodes[_walkNodesCount]._gridY1 = gridY + _walkDirY * _walkDirYIncr;
		if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
			_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
			_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
			++_walkDirXIncr;
			++_walkDirYIncr;
		} else if (_walkDeltaY - _walkDirYIncr > _walkDeltaX - _walkDirXIncr) {
			if (!_vm->isPointBlocked(_walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = 0;
				_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
				++_walkDirYIncr;
			} else if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
				_walkNodes[_walkNodesCount]._deltaY = 0;
				++_walkDirXIncr;
			} else
				return false;
		} else {
			if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
				_walkNodes[_walkNodesCount]._deltaY = 0;
				++_walkDirXIncr;
			} else if (!_vm->isPointBlocked(_walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = 0;
				_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
				++_walkDirYIncr;
			} else
				return false;
		}
		++_walkNodesCount;
	}

	while (_walkDirXIncr < _walkDeltaX) {
		_walkNodes[_walkNodesCount]._gridX1 = gridX + _walkDirX * _walkDirXIncr;
		_walkNodes[_walkNodesCount]._gridY1 = _walkDestY;
		if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkDestY)) {
			_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
			_walkNodes[_walkNodesCount]._deltaY = 0;
			++_walkDirXIncr;
			++_walkNodesCount;
		} else
			return false;
	}

	while (_walkDirYIncr < _walkDeltaY) {
		_walkNodes[_walkNodesCount]._gridX1 = _walkDestX;
		_walkNodes[_walkNodesCount]._gridY1 = gridY + _walkDirY * _walkDirYIncr;
		if (!_vm->isPointBlocked(_walkDestX, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
			_walkNodes[_walkNodesCount]._deltaX = 0;
			_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
			++_walkDirYIncr;
			++_walkNodesCount;
		} else
			return false;
	}

	return true;
}

bool PlayerGnap::findPath2(int gridX, int gridY, int index) {
	_walkNodesCount = index;
	_walkDirXIncr = 0;
	_walkDirYIncr = 0;
	_walkDeltaX = ABS(_walkDestX - gridX);
	_walkDeltaY = ABS(_walkDestY - gridY);

	if (_walkDeltaX)
		_walkDirX = (_walkDestX - gridX) / _walkDeltaX;
	else
		_walkDirX = 0;

	if (_walkDeltaY)
		_walkDirY = (_walkDestY - gridY) / _walkDeltaY;
	else
		_walkDirY = 0;

	while (_walkDeltaY < _walkDeltaX - _walkDirXIncr) {
		_walkNodes[_walkNodesCount]._gridX1 = gridX + _walkDirX * _walkDirXIncr;
		_walkNodes[_walkNodesCount]._gridY1 = gridY;
		if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, gridY)) {
			_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
			_walkNodes[_walkNodesCount]._deltaY = 0;
			++_walkDirXIncr;
			++_walkNodesCount;
		} else
			return false;
	}

	while (_walkDeltaX < _walkDeltaY - _walkDirYIncr) {
		_walkNodes[_walkNodesCount]._gridX1 = gridX;
		_walkNodes[_walkNodesCount]._gridY1 = gridY + _walkDirY * _walkDirYIncr;
		if (!_vm->isPointBlocked(gridX, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
			_walkNodes[_walkNodesCount]._deltaX = 0;
			_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
			++_walkDirYIncr;
			++_walkNodesCount;
		} else
			return false;
	}

	while (_walkDirXIncr < _walkDeltaX && _walkDirYIncr < _walkDeltaY) {
		_walkNodes[_walkNodesCount]._gridX1 = gridX + _walkDirX * _walkDirXIncr;
		_walkNodes[_walkNodesCount]._gridY1 = gridY + _walkDirY * _walkDirYIncr;
		if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
			_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
			_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
			++_walkDirXIncr;
			++_walkDirYIncr;
		} else if (_walkDeltaY - _walkDirYIncr > _walkDeltaX - _walkDirXIncr) {
			if (!_vm->isPointBlocked(_walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = 0;
				_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
				++_walkDirYIncr;
			} else if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
				_walkNodes[_walkNodesCount]._deltaY = 0;
				++_walkDirXIncr;
			} else
				return false;
		} else {
			if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
				_walkNodes[_walkNodesCount]._deltaY = 0;
				++_walkDirXIncr;
			} else if (!_vm->isPointBlocked(_walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = 0;
				_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
				++_walkDirYIncr;
			} else
				return false;
		}
		++_walkNodesCount;
	}

	while (_walkDirXIncr < _walkDeltaX) {
		_walkNodes[_walkNodesCount]._gridX1 = gridX + _walkDirX * _walkDirXIncr;
		_walkNodes[_walkNodesCount]._gridY1 = _walkDestY;
		if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkDestY)) {
			_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
			_walkNodes[_walkNodesCount]._deltaY = 0;
			++_walkDirXIncr;
			++_walkNodesCount;
		} else
			return false;
	}

	while (_walkDirYIncr < _walkDeltaY) {
		_walkNodes[_walkNodesCount]._gridX1 = _walkDestX;
		_walkNodes[_walkNodesCount]._gridY1 = gridY + _walkDirY * _walkDirYIncr;
		if (!_vm->isPointBlocked(_walkDestX, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
			_walkNodes[_walkNodesCount]._deltaX = 0;
			_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
			++_walkDirYIncr;
			++_walkNodesCount;
		} else
			return false;
	}

	return true;
}

bool PlayerGnap::findPath3(int gridX, int gridY) {
	int gridIncr = 1;
	bool done = false;

	while (!done && gridIncr < _vm->_gridMaxX) {
		if (!_vm->isPointBlocked(gridX + gridIncr, gridY) && findPath1(gridX + gridIncr, gridY, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = gridX + i;
				_walkNodes[i]._gridY1 = gridY;
				_walkNodes[i]._deltaX = 1;
				_walkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(gridX - gridIncr, gridY) && findPath1(gridX - gridIncr, gridY, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = gridX - i;
				_walkNodes[i]._gridY1 = gridY;
				_walkNodes[i]._deltaX = -1;
				_walkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(gridX, gridY + gridIncr) && findPath1(gridX, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = gridX;
				_walkNodes[i]._gridY1 = gridY + i;
				_walkNodes[i]._deltaX = 0;
				_walkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(gridX, gridY - gridIncr) && findPath1(gridX, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = gridX;
				_walkNodes[i]._gridY1 = gridY - i;
				_walkNodes[i]._deltaX = 0;
				_walkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(gridX + gridIncr, gridY + gridIncr) && findPath1(gridX + gridIncr, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = gridX + i;
				_walkNodes[i]._gridY1 = gridY + i;
				_walkNodes[i]._deltaX = 1;
				_walkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(gridX - gridIncr, gridY + gridIncr) && findPath1(gridX - gridIncr, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = gridX - i;
				_walkNodes[i]._gridY1 = gridY + i;
				_walkNodes[i]._deltaX = -1;
				_walkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(gridX + gridIncr, gridY - gridIncr) && findPath1(gridX + gridIncr, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = gridX + i;
				_walkNodes[i]._gridY1 = gridY - i;
				_walkNodes[i]._deltaX = 1;
				_walkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(gridX - gridIncr, gridY - gridIncr) && findPath1(gridX - gridIncr, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = gridX - i;
				_walkNodes[i]._gridY1 = gridY - i;
				_walkNodes[i]._deltaX = -1;
				_walkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(gridX + gridIncr, gridY) && findPath2(gridX + gridIncr, gridY, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = gridX + i;
				_walkNodes[i]._gridY1 = gridY;
				_walkNodes[i]._deltaX = 1;
				_walkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(gridX - gridIncr, gridY) && findPath2(gridX - gridIncr, gridY, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = gridX - i;
				_walkNodes[i]._gridY1 = gridY;
				_walkNodes[i]._deltaX = -1;
				_walkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(gridX, gridY + gridIncr) && findPath2(gridX, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = gridX;
				_walkNodes[i]._gridY1 = gridY + i;
				_walkNodes[i]._deltaX = 0;
				_walkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(gridX, gridY - gridIncr) && findPath2(gridX, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = gridX;
				_walkNodes[i]._gridY1 = gridY - i;
				_walkNodes[i]._deltaX = 0;
				_walkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(gridX + gridIncr, gridY + gridIncr) && findPath2(gridX + gridIncr, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = gridX + i;
				_walkNodes[i]._gridY1 = gridY + i;
				_walkNodes[i]._deltaX = 1;
				_walkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(gridX - gridIncr, gridY + gridIncr) && findPath2(gridX - gridIncr, gridY + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = gridX - i;
				_walkNodes[i]._gridY1 = gridY + i;
				_walkNodes[i]._deltaX = -1;
				_walkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(gridX + gridIncr, gridY - gridIncr) && findPath2(gridX + gridIncr, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = gridX + i;
				_walkNodes[i]._gridY1 = gridY - i;
				_walkNodes[i]._deltaX = 1;
				_walkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(gridX - gridIncr, gridY - gridIncr) && findPath2(gridX - gridIncr, gridY - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = gridX - i;
				_walkNodes[i]._gridY1 = gridY - i;
				_walkNodes[i]._deltaX = -1;
				_walkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		++gridIncr;
	}

	return done;
}

bool PlayerGnap::findPath4(int gridX, int gridY) {
	bool result = false;

	_walkNodesCount = 0;
	_walkDirXIncr = 0;
	_walkDirYIncr = 0;
	_walkDeltaX = ABS(_walkDestX - gridX);
	_walkDeltaY = ABS(_walkDestY - gridY);

	if (_walkDeltaX)
		_walkDirX = (_walkDestX - gridX) / _walkDeltaX;
	else
		_walkDirX = 0;

	if (_walkDeltaY)
		_walkDirY = (_walkDestY - gridY) / _walkDeltaY;
	else
		_walkDirY = 0;

	while (_walkDirXIncr < _walkDeltaX && _walkDirYIncr < _walkDeltaY) {
		_walkNodes[_walkNodesCount]._gridX1 = gridX + _walkDirX * _walkDirXIncr;
		_walkNodes[_walkNodesCount]._gridY1 = gridY + _walkDirY * _walkDirYIncr;
		if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
			_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
			_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
			++_walkDirXIncr;
			++_walkDirYIncr;
		} else if (_walkDeltaY - _walkDirYIncr > _walkDeltaX - _walkDirXIncr) {
			if (!_vm->isPointBlocked(_walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = 0;
				_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
				++_walkDirYIncr;
			} else if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
				_walkNodes[_walkNodesCount]._deltaY = 0;
				++_walkDirXIncr;
			} else {
				_walkDeltaX = _walkDirXIncr;
				_walkDeltaY = _walkDirYIncr;
				--_walkNodesCount;
			}
		} else {
			if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
				_walkNodes[_walkNodesCount]._deltaY = 0;
				++_walkDirXIncr;
			} else if (!_vm->isPointBlocked(_walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = 0;
				_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
				++_walkDirYIncr;
			} else {
				_walkDeltaX = _walkDirXIncr;
				_walkDeltaY = _walkDirYIncr;
				--_walkNodesCount;
			}
		}
		++_walkNodesCount;
	}

	while (_walkDirXIncr < _walkDeltaX) {
		_walkNodes[_walkNodesCount]._gridX1 = gridX + _walkDirX * _walkDirXIncr;
		_walkNodes[_walkNodesCount]._gridY1 = _walkDestY;
		if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkDestY)) {
			_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
			_walkNodes[_walkNodesCount]._deltaY = 0;
			++_walkDirXIncr;
			++_walkNodesCount;
		} else {
			_walkDeltaX = _walkDirXIncr;
		}
	}

	while (_walkDirYIncr < _walkDeltaY) {
		_walkNodes[_walkNodesCount]._gridX1 = _walkDestX;
		_walkNodes[_walkNodesCount]._gridY1 = gridY + _walkDirY * _walkDirYIncr;
		if (!_vm->isPointBlocked(_walkDestX, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
			_walkNodes[_walkNodesCount]._deltaX = 0;
			_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
			++_walkDirYIncr;
			++_walkNodesCount;
		} else {
			_walkDeltaY = _walkDirYIncr;
		}
	}

	if (gridX + _walkDirX * _walkDirXIncr != _walkDestX || gridY + _walkDirY * _walkDirYIncr != _walkDestY) {
		_walkDestX = gridX + _walkDirX * _walkDirXIncr;
		_walkDestY = gridY + _walkDirY * _walkDirYIncr;
		result = false;
	} else {
		result = true;
	}

	return result;
}

/******************************************************************************/

bool PlayerPlat::findPath1(int gridX, int gridY, int index) {
	_walkNodesCount = index;
	_walkDirXIncr = 0;
	_walkDirYIncr = 0;
	_walkDeltaX = ABS(_walkDestX - gridX);
	_walkDeltaY = ABS(_walkDestY - gridY);

	if (_walkDeltaX)
		_walkDirX = (_walkDestX - gridX) / _walkDeltaX;
	else
		_walkDirX = 0;

	if (_walkDeltaY)
		_walkDirY = (_walkDestY - gridY) / _walkDeltaY;
	else
		_walkDirY = 0;

	while (_walkDirXIncr < _walkDeltaX && _walkDirYIncr < _walkDeltaY) {
		_walkNodes[_walkNodesCount]._gridX1 = gridX + _walkDirX * _walkDirXIncr;
		_walkNodes[_walkNodesCount]._gridY1 = gridY + _walkDirY * _walkDirYIncr;
		if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
			_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
			_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
			++_walkDirXIncr;
			++_walkDirYIncr;
		} else if (_walkDeltaY - _walkDirYIncr > _walkDeltaX - _walkDirXIncr) {
			if (!_vm->isPointBlocked(_walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = 0;
				_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
				++_walkDirYIncr;
			} else if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
				_walkNodes[_walkNodesCount]._deltaY = 0;
				++_walkDirXIncr;
			} else
				return false;
		} else {
			if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
				_walkNodes[_walkNodesCount]._deltaY = 0;
				++_walkDirXIncr;
			} else if (!_vm->isPointBlocked(_walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = 0;
				_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
				++_walkDirYIncr;
			} else
				return false;
		}
		++_walkNodesCount;
	}

	while (_walkDirXIncr < _walkDeltaX) {
		_walkNodes[_walkNodesCount]._gridX1 = gridX + _walkDirX * _walkDirXIncr;
		_walkNodes[_walkNodesCount]._gridY1 = _walkDestY;
		if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkDestY)) {
			_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
			_walkNodes[_walkNodesCount]._deltaY = 0;
			++_walkDirXIncr;
			++_walkNodesCount;
		} else
			return false;
	}

	while (_walkDirYIncr < _walkDeltaY) {
		_walkNodes[_walkNodesCount]._gridX1 = _walkDestX;
		_walkNodes[_walkNodesCount]._gridY1 = gridY + _walkDirY * _walkDirYIncr;
		if (!_vm->isPointBlocked(_walkDestX, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
			_walkNodes[_walkNodesCount]._deltaX = 0;
			_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
			++_walkDirYIncr;
			++_walkNodesCount;
		} else
			return false;
	}

	return true;
}

bool PlayerPlat::findPath2(int gridX, int gridY, int index) {
	_walkNodesCount = index;
	_walkDirXIncr = 0;
	_walkDirYIncr = 0;
	_walkDeltaX = ABS(_walkDestX - gridX);
	_walkDeltaY = ABS(_walkDestY - gridY);

	if (_walkDeltaX)
		_walkDirX = (_walkDestX - gridX) / _walkDeltaX;
	else
		_walkDirX = 0;

	if (_walkDeltaY)
		_walkDirY = (_walkDestY - gridY) / _walkDeltaY;
	else
		_walkDirY = 0;

	while (_walkDeltaY < _walkDeltaX - _walkDirXIncr) {
		_walkNodes[_walkNodesCount]._gridX1 = gridX + _walkDirX * _walkDirXIncr;
		_walkNodes[_walkNodesCount]._gridY1 = gridY;
		if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, gridY)) {
			_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
			_walkNodes[_walkNodesCount]._deltaY = 0;
			++_walkDirXIncr;
			++_walkNodesCount;
		} else
			return false;
	}

	while (_walkDeltaX < _walkDeltaY - _walkDirYIncr) {
		_walkNodes[_walkNodesCount]._gridX1 = gridX;
		_walkNodes[_walkNodesCount]._gridY1 = gridY + _walkDirY * _walkDirYIncr;
		if (!_vm->isPointBlocked(gridX, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
			_walkNodes[_walkNodesCount]._deltaX = 0;
			_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
			++_walkDirYIncr;
			++_walkNodesCount;
		} else
			return false;
	}

	while (_walkDirXIncr < _walkDeltaX && _walkDirYIncr < _walkDeltaY) {
		_walkNodes[_walkNodesCount]._gridX1 = gridX + _walkDirX * _walkDirXIncr;
		_walkNodes[_walkNodesCount]._gridY1 = gridY + _walkDirY * _walkDirYIncr;
		if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
			_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
			_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
			++_walkDirXIncr;
			++_walkDirYIncr;
		} else if (_walkDeltaY - _walkDirYIncr > _walkDeltaX - _walkDirXIncr) {
			if (!_vm->isPointBlocked(_walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = 0;
				_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
				++_walkDirYIncr;
			} else if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
				_walkNodes[_walkNodesCount]._deltaY = 0;
				++_walkDirXIncr;
			} else
				return false;
		} else {
			if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
				_walkNodes[_walkNodesCount]._deltaY = 0;
				++_walkDirXIncr;
			} else if (!_vm->isPointBlocked(_walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = 0;
				_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
				++_walkDirYIncr;
			} else
				return false;
		}
		++_walkNodesCount;
	}

	while (_walkDirXIncr < _walkDeltaX) {
		_walkNodes[_walkNodesCount]._gridX1 = gridX + _walkDirX * _walkDirXIncr;
		_walkNodes[_walkNodesCount]._gridY1 = _walkDestY;
		if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkDestY)) {
			_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
			_walkNodes[_walkNodesCount]._deltaY = 0;
			++_walkDirXIncr;
			++_walkNodesCount;
		} else
			return false;
	}

	while (_walkDirYIncr < _walkDeltaY) {
		_walkNodes[_walkNodesCount]._gridX1 = _walkDestX;
		_walkNodes[_walkNodesCount]._gridY1 = gridY + _walkDirY * _walkDirYIncr;
		if (!_vm->isPointBlocked(_walkDestX, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
			_walkNodes[_walkNodesCount]._deltaX = 0;
			_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
			++_walkDirYIncr;
			++_walkNodesCount;
		} else
			return false;
	}

	return true;
}

bool PlayerPlat::findPath3(int gridX, int gridY) {
	int gridIncr = 1;
	bool done = false;

	while (!done && gridIncr < _vm->_gridMaxX) {
		if (!_vm->isPointBlocked(_pos.x + gridIncr, _pos.y) && findPath1(_pos.x + gridIncr, _pos.y, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = _pos.x + i;
				_walkNodes[i]._gridY1 = _pos.y;
				_walkNodes[i]._deltaX = 1;
				_walkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(_pos.x - gridIncr, _pos.y) && findPath1(_pos.x - gridIncr, _pos.y, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = _pos.x - i;
				_walkNodes[i]._gridY1 = _pos.y;
				_walkNodes[i]._deltaX = -1;
				_walkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(_pos.x, _pos.y + gridIncr) && findPath1(_pos.x, _pos.y + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = _pos.x;
				_walkNodes[i]._gridY1 = _pos.y + i;
				_walkNodes[i]._deltaX = 0;
				_walkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(_pos.x, _pos.y - gridIncr) && findPath1(_pos.x, _pos.y - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = _pos.x;
				_walkNodes[i]._gridY1 = _pos.y - i;
				_walkNodes[i]._deltaX = 0;
				_walkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(_pos.x + gridIncr, _pos.y + gridIncr) && findPath1(_pos.x + gridIncr, _pos.y + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = _pos.x + i;
				_walkNodes[i]._gridY1 = _pos.y + i;
				_walkNodes[i]._deltaX = 1;
				_walkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(_pos.x - gridIncr, _pos.y + gridIncr) && findPath1(_pos.x - gridIncr, _pos.y + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = _pos.x - i;
				_walkNodes[i]._gridY1 = _pos.y + i;
				_walkNodes[i]._deltaX = -1;
				_walkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(_pos.x + gridIncr, _pos.y - gridIncr) && findPath1(_pos.x + gridIncr, _pos.y - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = _pos.x + i;
				_walkNodes[i]._gridY1 = _pos.y - i;
				_walkNodes[i]._deltaX = 1;
				_walkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(_pos.x - gridIncr, _pos.y - gridIncr) && findPath1(_pos.x - gridIncr, _pos.y - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = _pos.x - i;
				_walkNodes[i]._gridY1 = _pos.y - i;
				_walkNodes[i]._deltaX = -1;
				_walkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(_pos.x + gridIncr, _pos.y) && findPath2(_pos.x + gridIncr, _pos.y, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = _pos.x + i;
				_walkNodes[i]._gridY1 = _pos.y;
				_walkNodes[i]._deltaX = 1;
				_walkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(_pos.x - gridIncr, _pos.y) && findPath2(_pos.x - gridIncr, _pos.y, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = _pos.x - i;
				_walkNodes[i]._gridY1 = _pos.y;
				_walkNodes[i]._deltaX = -1;
				_walkNodes[i]._deltaY = 0;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(_pos.x, _pos.y + gridIncr) && findPath2(_pos.x, _pos.y + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = _pos.x;
				_walkNodes[i]._gridY1 = _pos.y + i;
				_walkNodes[i]._deltaX = 0;
				_walkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(_pos.x, _pos.y - gridIncr) && findPath2(_pos.x, _pos.y - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = _pos.x;
				_walkNodes[i]._gridY1 = _pos.y - i;
				_walkNodes[i]._deltaX = 0;
				_walkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(_pos.x + gridIncr, _pos.y + gridIncr) && findPath2(_pos.x + gridIncr, _pos.y + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = _pos.x + i;
				_walkNodes[i]._gridY1 = _pos.y + i;
				_walkNodes[i]._deltaX = 1;
				_walkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(_pos.x - gridIncr, _pos.y + gridIncr) && findPath2(_pos.x - gridIncr, _pos.y + gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = _pos.x - i;
				_walkNodes[i]._gridY1 = _pos.y + i;
				_walkNodes[i]._deltaX = -1;
				_walkNodes[i]._deltaY = 1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(_pos.x + gridIncr, _pos.y - gridIncr) && findPath2(_pos.x + gridIncr, _pos.y - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = _pos.x + i;
				_walkNodes[i]._gridY1 = _pos.y - i;
				_walkNodes[i]._deltaX = 1;
				_walkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		if (!_vm->isPointBlocked(_pos.x - gridIncr, _pos.y - gridIncr) && findPath2(_pos.x - gridIncr, _pos.y - gridIncr, gridIncr)) {
			for (int i = 0; i < gridIncr; ++i) {
				_walkNodes[i]._gridX1 = _pos.x - i;
				_walkNodes[i]._gridY1 = _pos.y - i;
				_walkNodes[i]._deltaX = -1;
				_walkNodes[i]._deltaY = -1;
			}
			done = true;
			break;
		}
		++gridIncr;
	}

	return done;
}

bool PlayerPlat::findPath4(int gridX, int gridY) {
	bool result = false;

	_walkNodesCount = 0;
	_walkDirXIncr = 0;
	_walkDirYIncr = 0;
	_walkDeltaX = ABS(_walkDestX - gridX);
	_walkDeltaY = ABS(_walkDestY - gridY);

	if (_walkDeltaX)
		_walkDirX = (_walkDestX - gridX) / _walkDeltaX;
	else
		_walkDirX = 0;

	if (_walkDeltaY)
		_walkDirY = (_walkDestY - gridY) / _walkDeltaY;
	else
		_walkDirY = 0;

	while (_walkDirXIncr < _walkDeltaX && _walkDirYIncr < _walkDeltaY) {
		_walkNodes[_walkNodesCount]._gridX1 = gridX + _walkDirX * _walkDirXIncr;
		_walkNodes[_walkNodesCount]._gridY1 = gridY + _walkDirY * _walkDirYIncr;
		if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
			_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
			_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
			++_walkDirXIncr;
			++_walkDirYIncr;
		} else if (_walkDeltaY - _walkDirYIncr > _walkDeltaX - _walkDirXIncr) {
			if (!_vm->isPointBlocked(_walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = 0;
				_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
				++_walkDirYIncr;
			} else if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
				_walkNodes[_walkNodesCount]._deltaY = 0;
				++_walkDirXIncr;
			} else {
				_walkDeltaX = _walkDirXIncr;
				_walkDeltaY = _walkDirYIncr;
				--_walkNodesCount;
			}
		} else {
			if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
				_walkNodes[_walkNodesCount]._deltaY = 0;
				++_walkDirXIncr;
			} else if (!_vm->isPointBlocked(_walkNodes[_walkNodesCount]._gridX1, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
				_walkNodes[_walkNodesCount]._deltaX = 0;
				_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
				++_walkDirYIncr;
			} else {
				_walkDeltaX = _walkDirXIncr;
				_walkDeltaY = _walkDirYIncr;
				--_walkNodesCount;
			}
		}
		++_walkNodesCount;
	}

	while (_walkDirXIncr < _walkDeltaX) {
		_walkNodes[_walkNodesCount]._gridX1 = gridX + _walkDirX * _walkDirXIncr;
		_walkNodes[_walkNodesCount]._gridY1 = _walkDestY;
		if (!_vm->isPointBlocked(_walkDirX + _walkNodes[_walkNodesCount]._gridX1, _walkDestY)) {
			_walkNodes[_walkNodesCount]._deltaX = _walkDirX;
			_walkNodes[_walkNodesCount]._deltaY = 0;
			++_walkDirXIncr;
			++_walkNodesCount;
		} else {
			_walkDeltaX = _walkDirXIncr;
		}
	}

	while (_walkDirYIncr < _walkDeltaY) {
		_walkNodes[_walkNodesCount]._gridX1 = _walkDestX;
		_walkNodes[_walkNodesCount]._gridY1 = gridY + _walkDirY * _walkDirYIncr;
		if (!_vm->isPointBlocked(_walkDestX, _walkDirY + _walkNodes[_walkNodesCount]._gridY1)) {
			_walkNodes[_walkNodesCount]._deltaX = 0;
			_walkNodes[_walkNodesCount]._deltaY = _walkDirY;
			++_walkDirYIncr;
			++_walkNodesCount;
		} else {
			_walkDeltaY = _walkDirYIncr;
		}
	}

	if (gridX + _walkDirX * _walkDirXIncr != _walkDestX || gridY + _walkDirY * _walkDirYIncr != _walkDestY) {
		_walkDestX = gridX + _walkDirX * _walkDirXIncr;
		_walkDestY = gridY + _walkDirY * _walkDirYIncr;
		result = false;
	} else {
		result = true;
	}

	return result;
}

void PlayerPlat::makeRoom() {
	int rndGridX, rndGridY;
	do {
		rndGridY = _vm->getRandom(_vm->_gridMaxY);
		rndGridX = _vm->getRandom(_vm->_gridMaxX);
	} while (ABS(rndGridX - _pos.x) > 4 || ABS(rndGridY - _pos.y) > 3 ||
		_vm->isPointBlocked(rndGridX, rndGridY));
	walkTo(Common::Point(rndGridX, rndGridY), -1, -1, 1);
}

} // End of namespace Gnap
