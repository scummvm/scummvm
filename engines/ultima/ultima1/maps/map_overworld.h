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

#ifndef ULTIMA_ULTIMA1_MAP_MAP_OVERWORLD_H
#define ULTIMA_ULTIMA1_MAP_MAP_OVERWORLD_H

#include "ultima/ultima1/maps/map_base.h"

namespace Ultima {
namespace Ultima1 {
namespace Maps {

enum OverworldTile {
	OTILE_OCEAN = 0, OTILE_GRASS = 1, OTILE_WOODS = 2
};

class MapOverworld : public MapBase {
private:
	/**
	 * Load widget list for the map
	 */
	void loadWidgets();
public:
	MapOverworld(Ultima1Game *game, Ultima1Map *map) : MapBase(game, map) {}
	~MapOverworld() override {}

	/**
	 * Load the map
	 */
	void load(Shared::Maps::MapId mapId) override;

	/**
	 * Returns whether the map wraps around to the other side at it's edges (i.e. the overworld)
	 */
	bool isMapWrapped() const override { return true; }

	/**
	 * Shifts the viewport by a given delta
	 */
	void shiftViewport(const Point &delta) override;

	/**
	 * Get the viewport position
	 */
	Point getViewportPosition(const Point &viewportSize) override;

	/**
	 * Gets a point relative to the current position
	 */
	Point getDeltaPosition(const Point &delta) override;

	/**
	 * Perform an attack in a direction
	 * @param direction		Direction
	 * @param effectId		Sound effect to play
	 * @param maxDistance	Maximum distance in the given direction
	 * @param amount		Damage amount
	 * @param agility		Agility threshold
	 * @param widgetNa
	 */
	void attack(int direction, int effectId, uint maxDistance, uint amount, uint agility, const Common::String &hitWidget) override;

	/**
	 * Board a transport
	 */
	void board() override;

	/**
	 * Do an enter action
	 */
	void enter() override;

	/**
	 * Do an inform action
	 */
	void inform() override;

	/**
	 * Do an exit transport action
	 */
	void disembark() override;

	/**
	 * Get the lands number the player is currently within
	 */
	uint getLandsNumber() const;

	/**
	 * Adds a widget for the player being on foot, and sets it to the active player widget
	 */
	void addOnFoot();

	/**
	 * Get the number of active enemy vessels
	 */
	uint getEnemyVesselCount() const;
};

} // End of namespace Maps
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
