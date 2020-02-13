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

#include "ultima/ultima1/maps/map_overworld.h"
#include "ultima/ultima1/widgets/transport.h"
#include "ultima/ultima1/widgets/overworld_monster.h"
#include "ultima/ultima1/widgets/transport.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/resources.h"

namespace Ultima {
namespace Ultima1 {
namespace Maps {

void MapOverworld::load(Shared::Maps::MapId mapId) {
	Shared::Maps::MapBase::load(mapId);
	setDimensions(Point(168, 156));
	_tilesPerOrigTile = Point(1, 1);

	Shared::File f("map.bin");
	byte b;
	for (int y = 0; y < _size.y; ++y) {
		for (int x = 0; x < _size.x; x += 2) {
			b = f.readByte();
			_data[y][x] = b >> 4;
			_data[y][x + 1] = b & 0xf;
		}
	}

	// Load widgets
	loadWidgets();
}

void MapOverworld::loadWidgets() {
	// Note: the overworld player, transports, and monsters are persistant, so we only have to set up
	// the initial "on foot" transport the first time
	if (_widgets.empty()) {
		// Set up widget for the player
		_playerWidget = new Widgets::TransportOnFoot(_game, this);
		addWidget(_playerWidget);
	}
}

Point MapOverworld::getDeltaPosition(const Point &delta) {
	Point pt = _playerWidget->_position + delta;

	if (pt.x < 0)
		pt.x += _size.x;
	else if (pt.x >= _size.x)
		pt.x -= _size.x;
	if (pt.y < 0)
		pt.y += _size.y;
	else if (pt.y >= _size.y)
		pt.y -= _size.y;

	return pt;
}

Point MapOverworld::getViewportPosition(const Point &viewportSize) {
	Point &topLeft = _viewportPos._topLeft;

	if (!_viewportPos.isValid() || _viewportPos._size != viewportSize) {
		// Calculate the new position
		topLeft.x = _playerWidget->_position.x - (viewportSize.x - 1) / 2;
		topLeft.y = _playerWidget->_position.y - (viewportSize.y - 1) / 2;

		// Non-fixed map, so it wraps around the edges if necessary
		if (topLeft.x < 0)
			topLeft.x += width();
		else if (topLeft.x >= (int)width())
			topLeft.x -= width();

		if (topLeft.y < 0)
			topLeft.y += height();
		else if (topLeft.y >= (int)height())
			topLeft.y -= height();

		_viewportPos._mapId = _mapId;
		_viewportPos._size = viewportSize;
	}

	return topLeft;
}

void MapOverworld::shiftViewport(const Point &delta) {
	Point &topLeft = _viewportPos._topLeft;
	topLeft += delta;

	if (topLeft.x < 0)
		topLeft.x += width();
	else if (topLeft.x >= (int16)width())
		topLeft.x -= width();
	if (topLeft.y < 0)
		topLeft.y += height();
	else if (topLeft.y >= (int16)height())
		topLeft.y -= height();
}

void MapOverworld::board() {
	Maps::U1MapTile tile;
	getTileAt(getPosition(), &tile);
	Widgets::Transport *transport = dynamic_cast<Widgets::Transport *>(tile._widget);

	if (!dynamic_cast<Widgets::TransportOnFoot *>(_playerWidget)) {
		addInfoMsg(_game->_res->EXIT_CRAFT_FIRST, true, true);
		_game->playFX(1);
		_game->endOfTurn();
	} else if (!transport) {
		addInfoMsg(_game->_res->NOTHING_TO_BOARD, true, true);
		_game->playFX(1);
		_game->endOfTurn();
	} else {
		transport->board();
	}
}

void MapOverworld::enter() {
	Maps::U1MapTile tile;
	getTileAt(getPosition(), &tile);

	if (tile._locationNum == -1) {
		// Fall back to base unknown action
		MapBase::enter();
	} else {
		// Load the location
		Shared::Maps::Map *map = _game->getMap();
		map->load(tile._locationNum);

		// Add message for location having been entered
		addInfoMsg(_game->_res->ENTERING);
		addInfoMsg(map->getName());
	}
}

void MapOverworld::inform() {
	Maps::U1MapTile tile;
	getTileAt(getPosition(), &tile, false);

	addInfoMsg("");
	if (tile._locationNum != -1) {
		if (tile._locationNum < 33)
			addInfoMsg(Common::String::format("%s %s", _game->_res->THE_CITY_OF, _game->_res->LOCATION_NAMES[tile._locationNum - 1]));
		else
			addInfoMsg(_game->_res->LOCATION_NAMES[tile._locationNum - 1]);
	} else if (tile.isOriginalWater()) {
		addInfoMsg(_game->_res->YOU_ARE_AT_SEA);
	} else if (tile.isOriginalWoods()) {
		addInfoMsg(_game->_res->YOU_ARE_IN_WOODS);
	} else {
		addInfoMsg(_game->_res->YOU_ARE_IN_LANDS);
		addInfoMsg(_game->_res->LAND_NAMES[getLandsNumber()]);
	}
}

void MapOverworld::disembark() {
	Widgets::Transport *transport = dynamic_cast<Widgets::Transport *>(_playerWidget);

	if (transport) {
		addInfoMsg("");
		transport->disembark();
	} else {
		addInfoMsg(_game->_res->WHAT);
	}
}

uint MapOverworld::getLandsNumber() const {
	Point pt = getPosition();
	return (pt.y > 77 ? 2 : 0) + (pt.x > 83 ? 1 : 0);
}

void MapOverworld::addOnFoot() {
	_widgets.insert_at(0, Shared::Maps::MapWidgetPtr(new Widgets::TransportOnFoot(_game, this)));
	_playerWidget = _widgets[0].get();
}

uint MapOverworld::getEnemyVesselCount() const {
	uint total = 0;
	for (uint idx = 0; idx < _widgets.size(); ++idx) {
		if (dynamic_cast<Widgets::EnemyVessel *>(_widgets[idx].get()))
			++total;
	}

	return total;
}

void MapOverworld::attack(int direction, int effectId, uint maxDistance, uint amount, uint agility, const Common::String &hitWidget) {
	
}


} // End of namespace Maps
} // End of namespace Ultima1
} // End of namespace Ultima
