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

#include "ultima/ultima1/widgets/transport.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/maps/map_overworld.h"
#include "common/algorithm.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

TransportOnFoot::TransportOnFoot(Ultima1Game *game, Maps::MapBase *map) : OverworldWidget(game, map) {
	_name = game->_res->TRANSPORT_NAMES[0];
}

uint TransportOnFoot::getTileNum() const {
	return 8;
}

/*-------------------------------------------------------------------*/

Transport::Transport(Ultima1Game *game, Maps::MapBase *map, uint transportId) :
		OverworldWidget(game, map), _transportId(transportId) {
	_name = game->_res->TRANSPORT_NAMES[transportId];
}

void Transport::board() {
	assert(dynamic_cast<Widgets::TransportOnFoot *>(_map->_playerWidget));
	_map->removeWidget(_map->_playerWidget);
	_map->_playerWidget = this;

	addInfoMsg(Common::String::format(" %s", _name.c_str()));
	_game->endOfTurn();
}

void Transport::disembark() {
	Maps::U1MapTile tile;
	Point pt = _map->getPosition();
	Maps::MapOverworld *map = static_cast<Maps::MapOverworld *>(_map);

	// WORKAROUND: The original allowed dis-embarking if ground tiles were within two tiles of the transport.
	// It makes better sense to only allow if it's within one tile of the transport (i.e. the ground is adjacent)
	bool hasGround = false;
	for (int deltaY = -1; deltaY <= 1 && !hasGround; ++deltaY) {
		for (int deltaX = -1; deltaX <= 1 && !hasGround; ++deltaX) {
			_map->getTileAt(pt + Point(deltaX, deltaY), &tile);
			hasGround = tile._tileId != Maps::OTILE_OCEAN;
		}
	}

	if (tile._tileId > Maps::OTILE_WOODS || !hasGround) {
		addInfoMsg(getGame()->_res->CANT_LEAVE_IT_HERE);
	} else {
		map->addOnFoot();
	}
}

/*-------------------------------------------------------------------*/

EMPTY_MESSAGE_MAP(Horse, Transport);

Horse::Horse(Ultima1Game *game, Maps::MapBase *map) : Transport(game, map, 1) {
}

/*-------------------------------------------------------------------*/

EMPTY_MESSAGE_MAP(Cart, Transport);

Cart::Cart(Ultima1Game *game, Maps::MapBase *map) : Transport(game, map, 2) {
}

/*-------------------------------------------------------------------*/

EMPTY_MESSAGE_MAP(Raft, Transport);

Raft::Raft(Ultima1Game *game, Maps::MapBase *map) : Transport(game, map, 3) {
}

/*-------------------------------------------------------------------*/

EMPTY_MESSAGE_MAP(Frigate, Transport);

Frigate::Frigate(Ultima1Game *game, Maps::MapBase *map) : Transport(game, map, 4) {
}

Common::String Frigate::getWeaponsName() {
	Ultima1Game *game = static_cast<Ultima1Game *>(_game);
	return game->_res->TRANSPORT_WEAPONS[0];
}

/*-------------------------------------------------------------------*/

EMPTY_MESSAGE_MAP(Aircar, Transport);

Aircar::Aircar(Ultima1Game *game, Maps::MapBase *map) : Transport(game, map, 5) {
}

Common::String Aircar::getWeaponsName() {
	Ultima1Game *game = static_cast<Ultima1Game *>(_game);
	return game->_res->TRANSPORT_WEAPONS[1];
}

/*-------------------------------------------------------------------*/

EMPTY_MESSAGE_MAP(Shuttle, Transport);

Shuttle::Shuttle(Ultima1Game *game, Maps::MapBase *map) : Transport(game, map, 6),
		_space1(1000), _space2(1000) {
}

void Shuttle::synchronize(Common::Serializer &s) {
	Transport::synchronize(s);
	s.syncAsUint16LE(_space1);
	s.syncAsUint16LE(_space2);
}

/*-------------------------------------------------------------------*/

EMPTY_MESSAGE_MAP(TimeMachine, Transport);

TimeMachine::TimeMachine(Ultima1Game *game, Maps::MapBase *map) : Transport(game, map, 7) {
}

void TimeMachine::board() {
	// TODO
	Transport::board();
}


} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
