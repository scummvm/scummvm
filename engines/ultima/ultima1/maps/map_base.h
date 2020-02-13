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

#ifndef ULTIMA_ULTIMA1_MAPS_MAP_BASE_H
#define ULTIMA_ULTIMA1_MAPS_MAP_BASE_H

#include "ultima/shared/maps/map_base.h"

namespace Ultima {
namespace Ultima1 {

class Ultima1Game;

namespace Maps {

class Ultima1Map;

/**
 * Intermediate base class for Ultima 1 maps
 */
class MapBase : public Shared::Maps::MapBase {
private:
	/**
	 * Default unknown/question mark display
	 */
	void unknownAction();
protected:
	Ultima1Game *_game;
public:
	/**
	 * Constructor
	 */
	MapBase(Ultima1Game *game, Ultima1Map *map);

	/**
	 * Destructor
	 */
	~MapBase() override {}

	/**
	 * Gets a tile at a given position
	 */
	void getTileAt(const Point &pt, Shared::Maps::MapTile *tile, bool includePlayer = true) override;

	/**
	 * Instantiates a widget type by name
	 */
	Shared::Maps::MapWidget *createWidget(const Common::String &name) override;

	/**
	 * Default implementation for actions
	 */
	#define DEFAULT_ACTION(NAME) virtual void NAME() { unknownAction(); }
	DEFAULT_ACTION(drop)
	DEFAULT_ACTION(enter)
	DEFAULT_ACTION(get)
	DEFAULT_ACTION(hyperjump)
	DEFAULT_ACTION(inform)
	DEFAULT_ACTION(climb)
	DEFAULT_ACTION(open)
	DEFAULT_ACTION(steal)
	DEFAULT_ACTION(talk)
	DEFAULT_ACTION(unlock)
	DEFAULT_ACTION(view)
	DEFAULT_ACTION(disembark)

	/**
	 * Perform an attack
	 */
	virtual void attack(int direction, int effectId);

	/**
	 * Perform an attack in a direction
	 * @param direction		Direction
	 * @param effectId		Sound effect to play
	 * @param maxDistance	Maximum distance in the given direction
	 * @param amount		Damage amount
	 * @param agility		Agility threshold
	 * @param widgetNa
	 */
	virtual void attack(int direction, int effectId, uint maxDistance, uint amount, uint agility, const Common::String &hitWidget) = 0;

	/**
	 * Board a transport
	 */
	virtual void board();

	/**
	 * Cast a spell
	 */
	virtual void cast();

	/**
	 * Cast a specific spell
	 */
	void castSpell(uint spell) override;

	/**
	 * Handles dropping an amount of coins
	 */
	virtual void dropCoins(uint coins) {}
};

} // End of namespace Maps
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
