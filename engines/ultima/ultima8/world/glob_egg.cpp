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


#include "ultima/ultima8/world/glob_egg.h"
#include "ultima/ultima8/world/map_glob.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(GlobEgg)

GlobEgg::GlobEgg() {
}

GlobEgg::~GlobEgg() {
}


// Called when an item has entered the fast area
uint32 GlobEgg::enterFastArea() {
	uint32 coordmask = ~0x1FFU;
	unsigned int coordshift = 1;
	unsigned int offset = 1;
	if (GAME_IS_CRUSADER) {
		coordmask = ~0x3FFU;
		coordshift = 2;
		offset = 2;
	}

	// Expand it
	if (!hasFlags(FLG_FASTAREA)) {
		const MapGlob *glob = GameData::get_instance()->getGlob(_quality);
		if (!glob) return 0;

		for (const auto &globitem : glob->_contents) {
			Item *item = ItemFactory::createItem(globitem.shape, globitem.frame,
			                                     0,
			                                     FLG_DISPOSABLE | FLG_FAST_ONLY,
			                                     0, 0, 0, true);


			// calculate object's world position
			int32 itemx = (_x & coordmask) + (globitem.x << coordshift) + offset;
			int32 itemy = (_y & coordmask) + (globitem.y << coordshift) + offset;
			int32 itemz = _z + globitem.z;

			item->move(itemx, itemy, itemz);
		}
	}

	return Item::enterFastArea();
}

void GlobEgg::saveData(Common::WriteStream *ws) {
	Item::saveData(ws);
}

bool GlobEgg::loadData(Common::ReadStream *rs, uint32 version) {
	return Item::loadData(rs, version);
}

} // End of namespace Ultima8
} // End of namespace Ultima
