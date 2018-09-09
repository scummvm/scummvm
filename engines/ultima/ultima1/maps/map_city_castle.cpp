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

#include "ultima/ultima1/maps/map_city_castle.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/widgets/urban_player.h"
#include "ultima/ultima1/widgets/bard.h"
#include "ultima/ultima1/widgets/guard.h"
#include "ultima/ultima1/widgets/king.h"
#include "ultima/ultima1/widgets/princess.h"
#include "ultima/ultima1/widgets/wench.h"
#include "ultima/ultima1/widgets/merchant_armor.h"
#include "ultima/ultima1/widgets/merchant_grocer.h"
#include "ultima/ultima1/widgets/merchant_magic.h"
#include "ultima/ultima1/widgets/merchant_tavern.h"
#include "ultima/ultima1/widgets/merchant_transport.h"
#include "ultima/ultima1/widgets/merchant_weapons.h"

namespace Ultima {
namespace Ultima1 {
namespace Maps {

void MapCityCastle::load(Shared::Maps::MapId mapId) {
	clear();
	Shared::Maps::MapBase::load(mapId);

	setDimensions(Point(38, 18));
	_tilesPerOrigTile = Point(1, 1);
}

void MapCityCastle::clear() {
	Shared::Maps::MapBase::clear();
	_guardsHostile = false;
}

void MapCityCastle::loadWidgets() {
	// Set up widget for the player
	_playerWidget = new Widgets::UrbanPlayer(_game, this);
	addWidget(_playerWidget);

	for (int idx = 0; idx < 15; ++idx) {
		const int *lp = _game->_res->LOCATION_PEOPLE[_mapStyle * 15 + idx];
		if (lp[0] == -1)
			break;

		Widgets::Person *person;
		switch (lp[0]) {
		case 17:
			person = new Widgets::Guard(_game, this, lp[3]);
			break;
		case 19:
			person = new Widgets::Bard(_game, this, lp[3]);
			break;
		case 20:
			person = new Widgets::King(_game, this, lp[3]);
			break;
		case 21: {
			U1MapTile tile;
			getTileAt(Point(lp[1], lp[2]), &tile);

			switch (tile._tileId) {
			case 55:
				person = new Widgets::MerchantArmor(_game, this, lp[3]);
				break;
			case 57:
				person = new Widgets::MerchantGrocer(_game, this, lp[3]);
				break;
			case 59:
				person = new Widgets::MerchantWeapons(_game, this, lp[3]);
				break;
			case 60:
				person = new Widgets::MerchantMagic(_game, this, lp[3]);
				break;
			case 61:
				person = new Widgets::MerchantTavern(_game, this, lp[3]);
				break;
			case 62:
				person = new Widgets::MerchantTransport(_game, this, lp[3]);
				break;
			default:
				error("Invalid merchant");
			}
			break;
		}
		case 22:
			person = new Widgets::Princess(_game, this, lp[3]);
			break;
		case 50:
			person = new Widgets::Wench(_game, this, lp[3]);
			break;
		default:
			error("Unknown NPC type %d", lp[0]);
		}
		
		person->_position = Point(lp[1], lp[2]);
		addWidget(person);
	}
}

void MapCityCastle::getTileAt(const Point &pt, Shared::Maps::MapTile *tile, bool includePlayer) {
	MapBase::getTileAt(pt, tile, includePlayer);

	// Special handling for the cells indicating various merchant talk/steal positions
	if (tile->_tileDisplayNum >= 51)
		tile->_tileDisplayNum = 1;
}

Point MapCityCastle::getViewportPosition(const Point &viewportSize) {
	Point &topLeft = _viewportPos._topLeft;

	if (!_viewportPos.isValid() || _viewportPos._size != viewportSize) {
		// Calculate the new position
		topLeft.x = _playerWidget->_position.x - (viewportSize.x - 1) / 2;
		topLeft.y = _playerWidget->_position.y - (viewportSize.y - 1) / 2;

		// Fixed maps, so constrain top left corner so the map fills the viewport. This will accomodate
		// future renderings with more tiles, or greater tile size
		topLeft.x = CLIP((int)topLeft.x, 0, (int)(width() - viewportSize.x));
		topLeft.y = CLIP((int)topLeft.y, 0, (int)(height() - viewportSize.y));

		_viewportPos._mapId = _mapId;
		_viewportPos._size = viewportSize;
	}

	return topLeft;
}

void MapCityCastle::loadTownCastleData() {
	// Load the contents of the map
	Shared::File f("tcd.bin");
	f.seek(_mapStyle * 684);
	for (int x = 0; x < _size.x; ++x) {
		for (int y = 0; y < _size.y; ++y)
			_data[y][x] = f.readByte();
	}
}

void MapCityCastle::inform() {
	addInfoMsg("");
	addInfoMsg(_name);
}

void MapCityCastle::steal() {
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
		addInfoMsg(_game->_res->NOTHING_HERE);
		_game->playFX(1);
	}
}

/*-------------------------------------------------------------------*/

void MapCity::load(Shared::Maps::MapId mapId) {
	MapCityCastle::load(mapId);

	_mapStyle = (_mapId % 8) + 2;
	_mapIndex = _mapId;
	_name = Common::String::format("%s %s", _game->_res->THE_CITY_OF, _game->_res->LOCATION_NAMES[_mapId - 1]);

	loadTownCastleData();

	// Load up the widgets for the given map
	loadWidgets();
	setPosition(Common::Point(width() / 2, height() - 1));		// Start at bottom center edge of map
}

/*-------------------------------------------------------------------*/

void MapCastle::load(Shared::Maps::MapId mapId) {
	MapCityCastle::load(mapId);

	_mapIndex = _mapId - 33;
	_mapStyle = _mapIndex % 2;
	_name = _game->_res->LOCATION_NAMES[_mapId - 1];
	_castleKey = _game->getRandomNumber(255) & 1 ? 61 : 60;

	loadTownCastleData();

	// Set up door locks
	_data[_mapStyle ? 4 : 14][35] = CTILE_GATE;
	_data[_mapStyle ? 4 : 14][31] = CTILE_GATE;

	// Load up the widgets for the given map
	loadWidgets();
	setPosition(Common::Point(0, height() / 2));		// Start at center left edge of map
}

} // End of namespace Maps
} // End of namespace Ultima1
} // End of namespace Ultima
