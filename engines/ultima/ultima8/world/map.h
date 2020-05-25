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

#ifndef ULTIMA8_WORLD_MAP_H
#define ULTIMA8_WORLD_MAP_H

#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima8 {

class Item;

class Map {
	friend class CurrentMap;
public:
	explicit Map(uint32 mapnum);
	~Map();

	void clear();

	void loadNonFixed(Common::SeekableReadStream *rs);
	void loadFixed(Common::SeekableReadStream *rs);
	void unloadFixed();

	bool isEmpty() const {
		return _fixedItems.size() == 0 && _dynamicItems.size() == 0;
	}

	void save(Common::WriteStream *ods);
	bool load(Common::ReadStream *rs, uint32 version);

private:

	// load items from something formatted like 'fixed.dat'
	void loadFixedFormatObjects(Std::list<Item *> &itemlist,
								Common::SeekableReadStream *rs,
	                            uint32 extendedflags);

	// Add a fixed item to patch game data errors
	void addMapFix(uint32 shape, uint32 frame, int32 x, int32 y, int32 z);

	// Q: How should we store the items in a map.
	// It might make things more efficient if we order them by 'chunk'
	// (512x512). This would mean we need about 128x128 item lists.

	// It would probably be overkill to permanently maintain all these lists
	// for all maps, so we could only set them up for the current map.
	// (which makes me wonder if there should be a separate class for the
	// active map?)

	// (Note that we probably won't even have all items permanently stored,
	// since fixed items will be cached out most of the time)


	Std::list<Item *> _fixedItems;
	Std::list<Item *> _dynamicItems;

	uint32 _mapNum;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
