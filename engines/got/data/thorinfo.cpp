/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * as.syncAsUint32LE(with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/algorithm.h"
#include "got/data/thorinfo.h"
#include "got/game/back.h"

namespace Got {

void ThorInfo::clear() {
	_magic = 0;
	_keys = 0;
	_jewels = 0;
	_lastArea = 0;
	_lastScreen = 0;
	_lastIcon = 0;
	_lastDir = 0;
	_inventory = 0;
	_selectedItem = 0;
	_lastHealth = 0;
	_lastMagic = 0;
	_lastJewels = 0;
	_lastKeys = 0;
	_lastItem = 0;
	_lastInventory = 0;
	_level = 0;
	_score = 0;
	_lastScore = 0;
	_object = 0;
	_objectName = nullptr;
	_lastObject = 0;
	_lastObjectName = nullptr;
	_armor = 0;
	Common::fill(_filler, _filler + 65, 0);
}

void ThorInfo::sync(Common::Serializer &s) {
	uint32 ptr = 0;

	s.syncAsByte(_magic);
	s.syncAsByte(_keys);
	s.syncAsSint16LE(_jewels);
	s.syncAsByte(_lastArea);
	s.syncAsByte(_lastScreen);
	s.syncAsByte(_lastIcon);
	s.syncAsByte(_lastDir);
	s.syncAsSint16LE(_inventory);
	s.syncAsByte(_selectedItem);
	s.syncAsByte(_lastHealth);
	s.syncAsByte(_lastMagic);
	s.syncAsSint16LE(_lastJewels);
	s.syncAsByte(_lastKeys);
	s.syncAsByte(_lastItem);
	s.syncAsSint16LE(_lastInventory);
	s.syncAsByte(_level);
	s.syncAsUint32LE(_score);
	s.syncAsUint32LE(_lastScore);

	s.syncAsByte(_object);
	s.syncAsUint16LE(ptr);
	s.syncAsByte(_lastObject);
	s.syncAsUint16LE(ptr);

	s.syncAsByte(_armor);
	s.syncBytes(_filler, 65);

	if (s.isLoading()) {
		_objectName = (_object == 0) ? nullptr : OBJECT_NAMES[_object - 1];
		_lastObjectName = (_lastObject == 0) ? nullptr : OBJECT_NAMES[_lastObject - 1];
	}
}

} // namespace Got
