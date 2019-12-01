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

#include "ultima8/misc/pent_include.h"
#include "ultima8/world/map.h"
#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"
#include "ultima8/world/item_factory.h"
#include "ultima8/world/item.h"
#include "ultima8/world/container.h"
#include "ultima8/kernel/object_manager.h"
#include "ultima8/kernel/core_app.h"
#include "ultima8/games/game_info.h"

#include "ultima8/graphics/shape_info.h" // debugging only
#include "ultima8/games/game_data.h"
#include "ultima8/graphics/main_shape_archive.h"

namespace Ultima8 {

//#define DUMP_ITEMS

Map::Map(uint32 mapnum_)
	: mapnum(mapnum_) {

}


Map::~Map() {
	clear();
}

void Map::clear() {
	std::list<Item *>::iterator iter;

	for (iter = fixeditems.begin(); iter != fixeditems.end(); ++iter) {
		delete *iter;
	}
	fixeditems.clear();

	for (iter = dynamicitems.begin(); iter != dynamicitems.end(); ++iter) {
		delete *iter;
	}
	dynamicitems.clear();
}

void Map::loadNonFixed(IDataSource *ds) {
	loadFixedFormatObjects(dynamicitems, ds, 0);
}


// Utility function for fixing up map bugs: shift a coordinate to a
// different z without changing its on-screen position.
static void shiftCoordsToZ(int32 &x, int32 &y, int32 &z, int32 newz) {
	int32 zd = newz - z;

	x += 4 * zd;
	y += 4 * zd;

	z = newz;
}


void Map::loadFixed(IDataSource *ds) {
	loadFixedFormatObjects(fixeditems, ds, Item::EXT_FIXED);


	// U8 hack for missing ground tiles on map 25. See docs/u8bugs.txt
	if (GAME_IS_U8 && mapnum == 25) {
		// TODO
	}

	// U8 hack for missing ground/wall tiles on map 62. See docs/u8bugs.txt
	if (GAME_IS_U8 && mapnum == 62) {
		Item *item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                                     Item::EXT_FIXED, false);
		item->setLocation(16255, 6143, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(16639, 6143, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(16511, 6143, 48);
		fixeditems.push_back(item);


		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(15999, 6143, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(15871, 6143, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(15743, 6143, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(15615, 6143, 48);
		fixeditems.push_back(item);



		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(15999, 6015, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(15871, 6015, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(15743, 6015, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(15615, 6015, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(20095, 6911, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(20223, 6911, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(20095, 6783, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(20223, 6783, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(19839, 6655, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(19967, 6655, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(19839, 6527, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(19967, 6527, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(20095, 6527, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(19967, 6399, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(19839, 6399, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(301, 1, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(19711, 6399, 48);
		fixeditems.push_back(item);



		item = ItemFactory::createItem(497, 0, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(15487, 6271, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(497, 0, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(15359, 6271, 48);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(409, 32, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(14975, 6399, 0);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(409, 32, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(14975, 6015, 0);
		fixeditems.push_back(item);

		item = ItemFactory::createItem(409, 32, 0, 0, 0, 0,
		                               Item::EXT_FIXED, false);
		item->setLocation(15103, 6015, 0);
		fixeditems.push_back(item);
	}

	if (GAME_IS_U8 && mapnum == 49) {
		// Map 49 has some water tiles at the wrong z
		std::list<Item *>::iterator iter;

		for (iter = fixeditems.begin(); iter != fixeditems.end(); ++iter) {
			if ((*iter)->getShape() == 347 && (*iter)->getZ() == 96) {
				int32 x, y, z;
				(*iter)->getLocation(x, y, z);
				if ((x == 23007 && y == 21343) || (x == 23135 && y == 21471) ||
				        (x == 23135 && y == 21343)) {
					shiftCoordsToZ(x, y, z, 40);
					(*iter)->setLocation(x, y, z);
				}
			}
		}
	}

	if (GAME_IS_U8 && mapnum == 21) {
		// Map 21 has some ground and wall tiles at the wrong z
		std::list<Item *>::iterator iter;

		for (iter = fixeditems.begin(); iter != fixeditems.end(); ++iter) {
			int32 z = (*iter)->getZ();
			uint32 sh = (*iter)->getShape();
			if (z == 8 && (sh == 301 || sh == 31 || sh == 32)) {
				int32 x, y;
				(*iter)->getLocation(x, y, z);
				if ((x == 6783 || x == 6655) && (y == 15743 || y == 15615)) {
					shiftCoordsToZ(x, y, z, 16);
					(*iter)->setLocation(x, y, z);
				}
			}
		}
	}

	if (GAME_IS_U8 && mapnum == 5) {
		// Map 5 has some ground tiles at the wrong z
		std::list<Item *>::iterator iter;

		for (iter = fixeditems.begin(); iter != fixeditems.end(); ++iter) {
			if ((*iter)->getShape() == 71 && (*iter)->getFrame() == 8 && (*iter)->getZ() == 0) {
				int32 x, y, z;
				(*iter)->getLocation(x, y, z);
				if ((x == 9151 && y == 24127) || (x == 9279 && y == 23999) ||
				        (x == 9535 && y == 23615) || (x == 9151 && y == 23487) ||
				        (x == 10303 && y == 23487) || (x == 9919 && y == 23487) ||
				        (x == 10559 && y == 23487)) {
					shiftCoordsToZ(x, y, z, 48);
					(*iter)->setLocation(x, y, z);
				}
			}
		}
	}

}

void Map::unloadFixed() {
	std::list<Item *>::iterator iter;

	for (iter = fixeditems.begin(); iter != fixeditems.end(); ++iter) {
		delete *iter;
	}
	fixeditems.clear();
}

void Map::loadFixedFormatObjects(std::list<Item *> &itemlist, IDataSource *ds,
                                 uint32 extendedflags) {
	if (!ds) return;
	uint32 size = ds->getSize();
	if (size == 0) return;

	uint32 itemcount = size / 16;

	std::stack<Container *> cont;
	int contdepth = 0;

	for (uint32 i = 0; i < itemcount; ++i) {
		// These are ALL unsigned on disk
		int32 x = static_cast<int32>(ds->readX(2));
		int32 y = static_cast<int32>(ds->readX(2));
		int32 z = static_cast<int32>(ds->readX(1));

		if (GAME_IS_CRUSADER) {
			x *= 2;
			y *= 2;
		}

		uint32 shape = ds->read2();
		uint32 frame = ds->read1();
		uint16 flags = ds->read2();
		uint16 quality = ds->read2();
		uint16 npcnum = static_cast<uint16>(ds->read1());
		uint16 mapnum = static_cast<uint16>(ds->read1());
		uint16 next = ds->read2(); // do we need next for anything?

		// find container this item belongs to, if any.
		// the x coordinate stores the container-depth of this item,
		// so pop items from the container stack until we reach x,
		// or, if x is too large, the item is added to the top-level list
		while (contdepth != x && contdepth > 0) {
			cont.pop();
			contdepth--;
#ifdef DUMP_ITEMS
			pout << "---- Ending container ----" << std::endl;
#endif
		}

#ifdef DUMP_ITEMS
		pout << shape << "," << frame << ":\t(" << x << "," << y << "," << z << "),\t" << std::hex << flags << std::dec << ", " << quality << ", " << npcnum << ", " << mapnum << ", " << next << std::endl;
#endif

		Item *item = ItemFactory::createItem(shape, frame, quality, flags, npcnum,
		                                     mapnum, extendedflags, false);
		if (!item) {
			pout << shape << "," << frame << ":\t(" << x << "," << y << "," << z << "),\t" << std::hex << flags << std::dec << ", " << quality << ", " << npcnum << ", " << mapnum << ", " << next;

			ShapeInfo *info = GameData::get_instance()->getMainShapes()->
			                  getShapeInfo(shape);
			if (info) pout << ", family = " << info->family;
			pout << std::endl;

			pout << "Couldn't create item" << std::endl;
			continue;
		}
		item->setLocation(x, y, z);

		if (contdepth > 0) {
			cont.top()->addItem(item);
		} else {
			itemlist.push_back(item);
		}

		Container *c = p_dynamic_cast<Container *>(item);
		if (c) {
			// container, so prepare to read contents
			contdepth++;
			cont.push(c);
#ifdef DUMP_ITEMS
			pout << "---- Starting container ----" << std::endl;
#endif
		}
	}
}


void Map::save(ODataSource *ods) {
	ods->write4(static_cast<uint32>(dynamicitems.size()));

	std::list<Item *>::iterator iter;
	for (iter = dynamicitems.begin(); iter != dynamicitems.end(); ++iter) {
		(*iter)->save(ods);
	}
}


bool Map::load(IDataSource *ids, uint32 version) {
	uint32 itemcount = ids->read4();

	for (unsigned int i = 0; i < itemcount; ++i) {
		Object *obj = ObjectManager::get_instance()->loadObject(ids, version);
		Item *item = p_dynamic_cast<Item *>(obj);
		if (!item) return false;
		dynamicitems.push_back(item);
	}

	return true;
}

} // End of namespace Ultima8
