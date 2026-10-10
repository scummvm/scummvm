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

#ifndef ULTIMA3_VIEWS_LOCATION_MAP_H
#define ULTIMA3_VIEWS_LOCATION_MAP_H

#include "ultima/ultima3/logic/creature_logic.h"
#include "ultima/ultima3/logic/location_logic.h"
#include "ultima/ultima3/views/game.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

constexpr int VIEWPORT_TILES = 11;

/**
 * The world map, seen as an 11x11 area of tiles centred on the party.
 * Forests and mountains hide whatever is behind them
 */
class LocationMap : public Game {
private:
	LocationLogic _logic;
	CreatureLogic _creatures;

	// Set while a step is being taken, as opposed to some other command
	bool _moved = false;

	// Set when a fight has just begun, and when one has finished and the
	// turn it interrupted needs completing
	bool _fightStarted = false;
	bool _fightOver = false;

	/**
	 * Starts a fight with a creature
	 */
	void startFight(int creature);

	/**
	 * Fills in the tile numbers for the viewport, with the party's transport
	 * in the middle and hidden tiles blanked out
	 */
	void buildViewport(byte *tiles) const;

	void doMove(Direction dir, const char *label);
	void doPass();
	void doInvalid();
	void doBoard();
	void doEnter();
	void doQuitSave();
	void doAttack();
	void doExitVehicle();

protected:
	void drawViewport(GfxSurface &s) override;
	void idleTimeout() override;
	void endTurn() override;
	void processFrame() override;
	bool handleCommand(const KeypressMessage &msg) override;

public:
	LocationMap() : Game("LocationMap") {}
	~LocationMap() override {}

	bool msgFocus(const FocusMessage &msg) override;

	/**
	 * Has the party attack whatever is in a direction
	 */
	void attackDirection(Direction dir);
};

} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
