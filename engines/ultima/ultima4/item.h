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

#ifndef ULTIMA4_ITEM_H
#define ULTIMA4_ITEM_H

#include "ultima/ultima4/types.h"
#include "common/str.h"

namespace Ultima {
namespace Ultima4 {

class Map;
class Coords;

enum SearchCondition {
	SC_NONE         = 0x00,
	SC_NEWMOONS     = 0x01,
	SC_FULLAVATAR   = 0x02,
	SC_REAGENTDELAY = 0x04
};

struct ItemLocation {
	const char *_name;
	const char *_shortName;
	const char *_locationLabel;
	bool (*_isItemInInventory)(int item);
	void (*_putItemInInventory)(int item);
	void (*_useItem)(int item);
	int _data;
	unsigned char _conditions;
};

typedef void (*DestroyAllCreaturesCallback)(void);

void itemSetDestroyAllCreaturesCallback(DestroyAllCreaturesCallback callback);
const ItemLocation *itemAtLocation(const Map *map, const Coords &coords);
void itemUse(const Common::String &shortname);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
