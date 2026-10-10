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

#ifndef ULTIMA3_VIEWS_DUNGEON_MAP_H
#define ULTIMA3_VIEWS_DUNGEON_MAP_H

#include "ultima/ultima3/gfx/dungeon_view.h"
#include "ultima/ultima3/logic/dungeon_logic.h"
#include "ultima/ultima3/views/game.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

/**
 * Exploring a dungeon, seen from the party's point of view. The view
 * can only be made out when there's light to see it by
 */
class DungeonMap : public Game {
private:
	DungeonLogic _logic;
	Gfx::DungeonView _view;

	// Whether the picture of the view is up to date, and where it was taken from
	bool _viewValid = false;
	uint32 _viewKey = 0;

	// Set while the effects of a special square are being dealt with, which
	// don't count as a turn, and when a fight has ended and the turn is to be completed
	bool _special = false;
	bool _turnPending = false;

	/**
	 * Starts waiting for the next command
	 */
	void beginCommand();

	void doMove(bool forward);
	void doTurn(bool right);
	void doClimb();
	void doDescend();

protected:
	void drawViewport(GfxSurface &s) override;
	void drawLabels(GfxSurface &s) override;
	void idleTimeout() override;
	void endTurn() override;
	void processFrame() override;
	bool handleCommand(const KeypressMessage &msg) override;

public:
	DungeonMap() : Game("DungeonMap") {}
	~DungeonMap() override {}

	bool msgFocus(const FocusMessage &msg) override;
};

} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
