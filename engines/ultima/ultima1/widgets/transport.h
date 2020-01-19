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

#ifndef ULTIMA_ULTIMA1_WIDGETS_TRANSPORT_H
#define ULTIMA_ULTIMA1_WIDGETS_TRANSPORT_H

#include "ultima/ultima1/map/map.h"

namespace Ultima {
namespace Ultima1 {

class Ultima1Game;

namespace Widgets {

class Transport : public Shared::MapWidget {
protected:
	/**
	 * Gets the Ultima 1 game
	 */
	Ultima1Game *getGame() const;

	/**
	 * Gets the Ultima 1 map
	 */
	Map::Ultima1Map::MapBase *getMap() const;
public:
	/**
	 * Constructor
	 */
	Transport(Shared::Game *game, Shared::Map::MapBase *map) : Shared::MapWidget(game, map) {}

	/**
	 * Destructor
	 */
	virtual ~Transport() {}

	/**
	 * Returns true if the given transport type can move to a given position on the map
	 * @param destPos		Specified new position
	 */
	virtual bool canMoveTo(const Point &destPos) = 0;

	/**
	 * Moves to a given position
	 * @param destPos		Specified new position
	 * @returns				If true, the direction moved will be printed in the status area
	 */
	virtual bool moveTo(const Point &destPos) = 0;
};

class TransportOnFoot : public Transport {
private:
	/**
	 * Checks for whether a princess has been saved from a castle being left
	 */
	bool isPrincessSaved() const;

	/**
	 * Called for a princess being saved
	 */
	void princessSaved();
public:
	/**
	 * Constructor
	 */
	TransportOnFoot(Shared::Game *game, Shared::Map::MapBase *map) : Transport(game, map) {}

	/**
	 * Destructor
	 */
	virtual ~TransportOnFoot() {}

	/**
	 * Get the tile for the transport method
	 */
	virtual uint getTileNum() const;

	/**
	 * Returns true if the given transport type can move to a given position on the map
	 */
	virtual bool canMoveTo(const Point &destPos);

	/**
	 * Moves to a given position
	 * @param destPos		Specified new position
	 * @returns				If true, the direction moved will be printed in the status area
	 */
	virtual bool moveTo(const Point &destPos);
};

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
