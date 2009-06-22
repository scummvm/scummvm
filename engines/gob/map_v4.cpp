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

#include "common/stream.h"

#include "gob/gob.h"
#include "gob/map.h"
#include "gob/global.h"
#include "gob/goblin.h"
#include "gob/inter.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/mult.h"

namespace Gob {

Map_v4::Map_v4(GobEngine *vm) : Map_v2(vm) {
}

Map_v4::~Map_v4() {
}

void Map_v4::loadMapObjects(const char *avjFile) {
	uint8 wayPointsCount;
	int16 var;
	int16 id;
	int16 mapWidth, mapHeight;
	int16 tmp;
	byte *variables;
	byte *extData;
	uint32 tmpPos;
	uint32 passPos;

	var = _vm->_game->_script->readVarIndex();
	variables = _vm->_inter->_variables->getAddressOff8(var);

	id = _vm->_game->_script->readInt16();

	if (((uint16) id) >= 65520) {
		warning("Woodruff Stub: loadMapObjects ID >= 65520");
		return;
	} else if (id == -1) {
		_passMap = (int8 *) _vm->_inter->_variables->getAddressOff8(var);
		return;
	}

	extData = _vm->_game->loadExtData(id, 0, 0);
	Common::MemoryReadStream mapData(extData, 4294967295U);

	_widthByte = mapData.readByte();
	if (_widthByte == 4) {
		_screenWidth = 640;
		_screenHeight = 400;
	} else if (_widthByte == 3) {
		_screenWidth = 640;
		_screenHeight = 200;
	} else {
		_screenWidth = 320;
		_screenHeight = 200;
	}

	_wayPointsCount = mapData.readByte();
	_tilesWidth = mapData.readSint16LE();
	_tilesHeight = mapData.readSint16LE();

	_bigTiles = !(_tilesHeight & 0xFF00);
	_tilesHeight &= 0xFF;

	if (_widthByte == 4) {
		_screenWidth = mapData.readSint16LE();
		_screenHeight = mapData.readSint16LE();
	}

	_mapWidth = _screenWidth / _tilesWidth;
	_mapHeight = _screenHeight / _tilesHeight;

	passPos = mapData.pos();
	mapData.skip(_mapWidth * _mapHeight);

	if (*extData == 1)
		wayPointsCount = _wayPointsCount = 40;
	else
		wayPointsCount = _wayPointsCount == 0 ? 1 : _wayPointsCount;

	delete[] _wayPoints;
	_wayPoints = new Point[wayPointsCount];
	for (int i = 0; i < _wayPointsCount; i++) {
		_wayPoints[i].x = mapData.readSByte();
		_wayPoints[i].y = mapData.readSByte();
		_wayPoints[i].notWalkable = mapData.readSByte();
	}

	if (_widthByte == 4)
		_mapWidth = VAR(17);

	_passWidth = _mapWidth;

	// In the original asm, this writes byte-wise into the variables-array
	tmpPos = mapData.pos();
	mapData.seek(passPos);
	if ((variables != 0) &&
	    (variables != _vm->_inter->_variables->getAddressOff8(0))) {

		_passMap = (int8 *) variables;
		mapHeight = _screenHeight / _tilesHeight;
		mapWidth = _screenWidth / _tilesWidth;

		for (int i = 0; i < mapHeight; i++) {
			for (int j = 0; j < mapWidth; j++)
				setPass(j, i, mapData.readSByte());
			_vm->_inter->_variables->getAddressOff8(var + i * _passWidth);
		}
	}
	mapData.seek(tmpPos);

	tmp = mapData.readSint16LE();
	mapData.skip(tmp * 14);
	tmp = mapData.readSint16LE();
	mapData.skip(tmp * 14 + 28);
	tmp = mapData.readSint16LE();
	mapData.skip(tmp * 14);

	_vm->_goblin->_gobsCount = tmp;
	for (int i = 0; i < _vm->_goblin->_gobsCount; i++)
		loadGoblinStates(mapData, i);

	_vm->_goblin->_soundSlotsCount = _vm->_game->_script->readInt16();
	for (int i = 0; i < _vm->_goblin->_soundSlotsCount; i++)
		_vm->_goblin->_soundSlots[i] = _vm->_inter->loadSound(1);

	delete[] extData;
}

} // End of namespace Gob
