
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

#ifndef ULTIMA2_LOGIC_OVERWORLD_LOGIC_H
#define ULTIMA2_LOGIC_OVERWORLD_LOGIC_H

#include "ultima/ultima1/logic/logic.h"
#include "ultima/ultima1/data/map.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

class OverworldLogic : public Logic {
private:
	/**
	 * Enter a location
	 */
	void enter();

	/**
	 * Pass a turn
	 */
	void pass();

	/**
	 * Check for movement
	 */
	bool moveCheck(Data::Direction dir);

	/**
	 * Handle movement
	 */
	void move(Data::Direction dir);

	/**
	 * Get the X co-ordinate for a tile within the map, offset from the left edge of the visible map area
	 * @param xp	X offset within the window
	 * @return		X-coordinate within the map
	*/
	int getViewportX(int xp) const;

	/**
	 * Get the Y co-ordinate for a tile within the map, offset from the top edge of the visible map area
	 * @param yp	Y offset within the window
	 * @return		Y-coordinate within the map
	 */
	int getViewportY(int yp) const;

	/**
	 * Returns a tile or creature/entity at a given co-ordinate
	 */
	int getTileAt(int x, int y, int creatureIndex = 1) const;

	/**
	 * Impassable tile
	 */
	void impassable(int reason);

	/**
	 * Move left
	 */
	void moveLeft();

	/**
	 * Move right
	 */
	void moveRight();

	/**
	 * Move up
	 */
	void moveUp();

	/**
	 * Move down
	 */
	void moveDown();

	/**
	 * Called when the player changes their current continent
	 */
	void continentChanged(int oldContinent);

	/**
	 * Called after an action is done
	 */
	void endOfTurn();

	/**
	 * Generate monsters for the overworld
	 */
	void generateCreatures();

	/**
	 * Update overworld creatures
	 */
	void updateCreatures();

	/**
	 * Reduce the food the player has
	 */
	void reduceFood();

public:
	OverworldLogic();
	~OverworldLogic() override {
	}

	void action(int action) override;
	void keypress(Common::KeyCode keycode);
};

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima

#endif
