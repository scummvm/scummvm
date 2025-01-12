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

struct THOR_INFO {
	byte magic = 0;
	byte keys = 0;
	int jewels = 0;
	byte last_area = 0;
	byte last_screen = 0;
	byte last_icon = 0;
	byte last_dir = 0;
	int inventory = 0;
	byte item = 0; //currently selected item
	byte last_health = 0;
	byte last_magic = 0;
	int last_jewels = 0;
	byte last_keys = 0;
	byte last_item = 0;
	int last_inventory = 0;
	byte level = 0; //current level (1,2,3)
	long score = 0;
	long last_score = 0;
	byte object = 0;
	const char *object_name = nullptr;
	byte last_object = 0;
	const char *last_object_name = nullptr;
	byte armor = 0;
	byte future[65] = {};

	void clear();
	void sync(Common::Serializer &s);
};

} // namespace Got

#endif
