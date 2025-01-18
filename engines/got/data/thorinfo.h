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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GOT_DATA_THOR_INFO_H
#define GOT_DATA_THOR_INFO_H

#include "common/serializer.h"

namespace Got {

struct ThorInfo {
	byte _magic = 0;
	byte _keys = 0;
	int _jewels = 0;
	byte _lastArea = 0;
	byte _lastScreen = 0;
	byte _lastIcon = 0;
	byte _lastDir = 0;
	int _inventory = 0;
	byte _selectedItem = 0; //currently selected item
	byte _lastHealth = 0;
	byte _lastMagic = 0;
	int _lastJewels = 0;
	byte _lastKeys = 0;
	byte _lastItem = 0;
	int _lastInventory = 0;
	byte _level = 0; //current level (1,2,3)
	long _score = 0;
	long _lastScore = 0;
	byte _object = 0;
	const char *_objectName = nullptr;
	byte _lastObject = 0;
	const char *_lastObjectName = nullptr;
	byte _armor = 0;
	byte _filler[65] = {};

	void clear();
	void sync(Common::Serializer &s);
};

} // namespace Got

#endif
