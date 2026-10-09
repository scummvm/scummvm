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

#ifndef ULTIMA3_VIEWS_COMBAT_MAP_H
#define ULTIMA3_VIEWS_COMBAT_MAP_H

#include "ultima/ultima3/views/game.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

/**
 * A fight on a small arena. The party members each take a turn in order,
 * followed by the monsters, until one side is beaten
 */
class CombatMap : public Game {
private:
	// What happens once the events from a command have been played
	enum Next {
		NOTHING,
		NEXT_PLAYER,
		NEXT_ROUND
	};

	bool _started = false;
	Next _next = NOTHING;

	// A tile shown over the arena for a frame
	bool _overlay = false;
	int _overlayX = 0, _overlayY = 0, _overlayTile = 0;

	/**
	 * Starts a round, in which each party member and then the monsters act
	 */
	void beginRound();

	/**
	 * Starts the turn of the party member whose turn it is, passing over
	 * any who can't act
	 */
	void beginTurn();

	/**
	 * Moves on from a turn that has taken place
	 */
	void advanceTurn();

	void monstersTurn();
	void victory();
	void doMove(int dx, int dy, const char *label);
	void doNegateTime();
	void doAttack();

protected:
	void drawViewport(GfxSurface &s) override;
	void idleTimeout() override;
	void endTurn() override;
	void processFrame() override;
	bool isWaiting() const override;
	bool handleCommand(const KeypressMessage &msg) override;

public:
	CombatMap() : Game("CombatMap") {}
	~CombatMap() override {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
