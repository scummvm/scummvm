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

#include "ultima/ultima.h"
#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/misc/point3.h"
#include "ultima/ultima8/world/map.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/container.h"
#include "ultima/ultima8/world/coord_utils.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/main_shape_archive.h"

namespace Ultima {
namespace Ultima8 {

//#define DUMP_ITEMS

Map::Map(uint32 mapNum) : _mapNum(mapNum) {
}


Map::~Map() {
	clear();
}

void Map::clear() {
	for (auto *item : _fixedItems) {
		delete item;
	}
	_fixedItems.clear();

	for (auto *item : _dynamicItems) {
		delete item;
	}
	_dynamicItems.clear();
}

void Map::loadNonFixed(Common::SeekableReadStream *rs) {
	loadFixedFormatObjects(_dynamicItems, rs, 0);
}


// Utility function for fixing up map bugs: shift a coordinate to a
// different z without changing its on-screen position.
static void shiftCoordsToZ(int32 &x, int32 &y, int32 &z, int32 newz) {
	int32 zd = newz - z;

	x += 4 * zd;
	y += 4 * zd;

	z = newz;
}


void Map::addMapFix(uint32 shape, uint32 frame, int32 x, int32 y, int32 z) {
	Item *item = ItemFactory::createItem(shape, frame, 0, 0, 0, 0,
										 Item::EXT_FIXED, false);
	item->setLocation(x, y, z);
	_fixedItems.push_back(item);
}


void Map::loadFixed(Common::SeekableReadStream *rs) {
	loadFixedFormatObjects(_fixedItems, rs, Item::EXT_FIXED);

	// WORKAROUND - add some missing map tiles from game data

	// U8 hack for missing ground tiles on map 25. See docs/u8bugs.txt
	if (GAME_IS_U8 && _mapNum == 25) {
		// TODO
	}

	// U8 hack for missing ground tiles on map 7 (north road).
	if (GAME_IS_U8 && _mapNum == 7) {
		addMapFix(301, 1, 2815, 25727, 8);
		addMapFix(301, 1, 9983, 21157, 8);
		addMapFix(301, 1, 13183, 16511, 8);
	}

	// Upper Catacombs after using the skull (ScummVM bug #12097)
	// Not a perfect fix (still a few pixels off) but reduces chance
	// of bug happening
	if (GAME_IS_U8 && _mapNum == 50) {
		addMapFix(34, 7, 16127, 6143, 0);
	}

	// U8 hack for missing ground/wall tiles on map 62. See docs/u8bugs.txt
	if (GAME_IS_U8 && _mapNum == 62) {
		addMapFix(301, 1, 16255, 6143, 48);
		addMapFix(301, 1, 16639, 6143, 48);
		addMapFix(301, 1, 16511, 6143, 48);
		addMapFix(301, 1, 15999, 6143, 48);
		addMapFix(301, 1, 15871, 6143, 48);
		addMapFix(301, 1, 15743, 6143, 48);
		addMapFix(301, 1, 15615, 6143, 48);
		addMapFix(301, 1, 15999, 6015, 48);
		addMapFix(301, 1, 15871, 6015, 48);
		addMapFix(301, 1, 15743, 6015, 48);
		addMapFix(301, 1, 15615, 6015, 48);
		addMapFix(301, 1, 20095, 6911, 48);
		addMapFix(301, 1, 20223, 6911, 48);
		addMapFix(301, 1, 20095, 6783, 48);
		addMapFix(301, 1, 20223, 6783, 48);
		addMapFix(301, 1, 19839, 6655, 48);
		addMapFix(301, 1, 19967, 6655, 48);
		addMapFix(301, 1, 19839, 6527, 48);
		addMapFix(301, 1, 19967, 6527, 48);
		addMapFix(301, 1, 20095, 6527, 48);
		addMapFix(301, 1, 19967, 6399, 48);
		addMapFix(301, 1, 19839, 6399, 48);
		addMapFix(301, 1, 19711, 6399, 48);
		addMapFix(497, 0, 15487, 6271, 48);
		addMapFix(497, 0, 15359, 6271, 48);
		addMapFix(409, 32, 14975, 6399, 0);
		addMapFix(409, 32, 14975, 6015, 0);
		addMapFix(409, 32, 15103, 6015, 0);
	}

	if (GAME_IS_U8 && _mapNum == 49) {
		// Map 49 has some water tiles at the wrong z
		for (auto *item : _fixedItems) {
			if (item->getShape() == 347 && item->getZ() == 96) {
				Point3 pt = item->getLocation();
				if ((pt.x == 23007 && pt.y == 21343) || (pt.x == 23135 && pt.y == 21471) ||
				        (pt.x == 23135 && pt.y == 21343)) {
					shiftCoordsToZ(pt.x, pt.y, pt.z, 40);
					item->setLocation(pt);
				}
			}
		}
	}

	if (GAME_IS_U8 && _mapNum == 21) {
		// Map 21 has some ground and wall tiles at the wrong z
		for (auto *item : _fixedItems) {
			int32 z = item->getZ();
			uint32 sh = item->getShape();
			if (z == 8 && (sh == 301 || sh == 31 || sh == 32)) {
				Point3 pt = item->getLocation();
				if ((pt.x == 6783 || pt.x == 6655) && (pt.y == 15743 || pt.y == 15615)) {
					shiftCoordsToZ(pt.x, pt.y, pt.z, 16);
					item->setLocation(pt);
				}
			}
		}
	}

	if (GAME_IS_U8 && _mapNum == 5) {
		// Map 5 has some ground tiles at the wrong z
		for (auto *item : _fixedItems) {
			if (item->getShape() == 71 && item->getFrame() == 8 && item->getZ() == 0) {
				Point3 pt = item->getLocation();
				if ((pt.x == 9151 && pt.y == 24127) || (pt.x == 9279 && pt.y == 23999) ||
				        (pt.x == 9535 && pt.y == 23615) || (pt.x == 9151 && pt.y == 23487) ||
				        (pt.x == 10303 && pt.y == 23487) || (pt.x == 9919 && pt.y == 23487) ||
				        (pt.x == 10559 && pt.y == 23487)) {
					shiftCoordsToZ(pt.x, pt.y, pt.z, 48);
					item->setLocation(pt);
				}
			}
		}
	}

}

void Map::unloadFixed() {
	for (auto *item : _fixedItems) {
		delete item;
	}
	_fixedItems.clear();
}

void Map::loadFixedFormatObjects(Std::list<Item *> &itemlist,
								 Common::SeekableReadStream *rs,
								 uint32 extendedflags) {
	if (!rs) return;
	uint32 size = rs->size();
	if (size == 0) return;

	uint32 itemcount = size / 16;

	Common::Stack<Container *> cont;
	int contdepth = 0;

	for (uint32 i = 0; i < itemcount; ++i) {
		// These are ALL unsigned on disk
		int32 x = static_cast<int32>(rs->readUint16LE());
		int32 y = static_cast<int32>(rs->readUint16LE());
		int32 z = static_cast<int32>(rs->readByte());

		World_FromUsecodeXY(x, y);

		uint32 shape = rs->readUint16LE();
		uint32 frame = rs->readByte();
		uint16 flags = rs->readUint16LE();
		uint16 quality = rs->readUint16LE();
		uint16 npcNum = static_cast<uint16>(rs->readByte());
		uint16 mapNum = static_cast<uint16>(rs->readByte());
		uint16 next = rs->readUint16LE(); // do we need next for anything?

		// find container this item belongs to, if any.
		// the x coordinate stores the container-depth of this item,
		// so pop items from the container stack until we reach x,
		// or, if x is too large, the item is added to the top-level list
		while (contdepth != x && contdepth > 0) {
			cont.pop();
			contdepth--;
#ifdef DUMP_ITEMS
			debugC(kDebugObject, "---- Ending container ----");
#endif
		}

#ifdef DUMP_ITEMS
		debugC(kDebugObject, "%u,%u:\t(%d, %d, %d),\t%x, %u, %u, %u, %u",
			shape , frame, x, y, z, flags, quality, npcNum, mapNum, next);
#endif

		Item *item = ItemFactory::createItem(shape, frame, quality, flags, npcNum,
		                                     mapNum, extendedflags, false);
		if (!item) {
			warning("Couldn't create item: %u,%u:\t(%d, %d, %d),\t%x, %u, %u, %u, %u",
				shape, frame, x, y, z, flags, quality, npcNum, mapNum, next);
			continue;
		}
		item->setLocation(x, y, z);

		if (contdepth > 0) {
			cont.top()->addItem(item);
		} else {
			itemlist.push_back(item);
		}

		Container *c = dynamic_cast<Container *>(item);
		if (c) {
			// container, so prepare to read contents
			contdepth++;
			cont.push(c);
#ifdef DUMP_ITEMS
			debugC(kDebugObject, "---- Starting container ----");
#endif
		}
	}
}


void Map::save(Common::WriteStream *ws) {
	ws->writeUint32LE(static_cast<uint32>(_dynamicItems.size()));

	for (auto *item : _dynamicItems) {
		ObjectManager::get_instance()->saveObject(ws, item);
	}
}


bool Map::load(Common::ReadStream *rs, uint32 version) {
	uint32 itemcount = rs->readUint32LE();

	// Integrity check
	if (itemcount > 65536) {
		warning("improbable item count in map data: %d", itemcount);
		return false;
	}

	for (unsigned int i = 0; i < itemcount; ++i) {
		Object *obj = ObjectManager::get_instance()->loadObject(rs, version);
		Item *item = dynamic_cast<Item *>(obj);
		if (!item) return false;
		_dynamicItems.push_back(item);
	}

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
