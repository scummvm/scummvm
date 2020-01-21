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

#include "ultima/ultima1/maps/map_base.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/maps/map_dungeon.h"
#include "ultima/ultima1/maps/map_city_castle.h"
#include "ultima/ultima1/maps/map_overworld.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/widgets/merchant_armor.h"
#include "ultima/ultima1/widgets/merchant_grocer.h"
#include "ultima/ultima1/widgets/merchant_magic.h"
#include "ultima/ultima1/widgets/merchant_tavern.h"
#include "ultima/ultima1/widgets/merchant_transport.h"
#include "ultima/ultima1/widgets/merchant_weapons.h"

namespace Ultima {
namespace Ultima1 {
namespace Maps {

MapBase::MapBase(Ultima1Game *game, Ultima1Map *map) : Shared::Maps::MapBase(game, map), _game(game) {
}

void MapBase::getTileAt(const Point &pt, Shared::Maps::MapTile *tile) {
	Shared::Maps::MapBase::getTileAt(pt, tile);

	// Extended properties to set if an Ultima 1 map tile structure was passed in
	U1MapTile *mapTile = dynamic_cast<U1MapTile *>(tile);
	if (mapTile) {
		GameResources *res = _game->_res;
		mapTile->setMap(this);

		// Check for a location at the given position
		mapTile->_locationNum = -1;
		if (dynamic_cast<MapOverworld *>(this)) {
			for (int idx = 0; idx < LOCATION_COUNT; ++idx) {
				if (pt.x == res->LOCATION_X[idx] && pt.y == res->LOCATION_Y[idx]) {
					mapTile->_locationNum = idx + 1;
					break;
				}
			}
		}
	}
}

void MapBase::steal() {
	U1MapTile tile;
	getTileAt(getPosition(), &tile);

	// Scan for the correct merchant depending on the tile player is on
	Widgets::Merchant *merchant = nullptr;
	switch (tile._tileId) {
	case 55:
		merchant = dynamic_cast<Widgets::MerchantArmor *>(_widgets.findByClass(Widgets::MerchantArmor::type()));
		break;
	case 57:
		merchant = dynamic_cast<Widgets::MerchantGrocer *>(_widgets.findByClass(Widgets::MerchantGrocer::type()));
		break;
	case 59:
		merchant = dynamic_cast<Widgets::MerchantWeapons *>(_widgets.findByClass(Widgets::MerchantWeapons::type()));
		break;
	default:
		break;
	}

	if (merchant) {
		// Found a merchant, so call their steal handler
		merchant->steal();
	} else {
		addInfoMsg("?");
		_game->playFX(1);
	}
}

void MapBase::talk() {
	addInfoMsg("?");
	_game->playFX(1);
}

} // End of namespace Maps
} // End of namespace Ultima1
} // End of namespace Ultima
