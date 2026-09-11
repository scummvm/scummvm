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

#ifndef ULTIMA1_VIEWS_INTERACTIONS_DEAD_H
#define ULTIMA1_VIEWS_INTERACTIONS_DEAD_H

#include "ultima/ultima1/views/interactions/interaction.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

using namespace Ultima::Shared::Messages;

/**
 * Shown when the player's hit points reach zero. Plays out the original's
 * resurrection sequence: shows the skull glyph and "<name>, thou art dead",
 * pauses briefly, then "Attempting resurrection!" and animates hits and
 * food back up to 99 each, one point at a time. Finishes by stripping the
 * player's weapons and any ridden transport, moving them to a random spot
 * on the continent they died on, and returning to the overworld.
 *
 * The whole sequence plays out automatically via tick() - no player input
 * is needed, or accepted, while it's running.
 */
class Dead : public Interaction {
private:
	enum Phase {
		PHASE_INTRO,
		PHASE_HITS,
		PHASE_FOOD
	};

	Phase _phase = PHASE_INTRO;
	int _delayCtr = 0;

	/**
	 * Draws the skull glyph shown in the map area while the player is dead
	 */
	void drawDeathGraphic();

	/**
	 * Removes any overworld creatures on a given continent. Called for
	 * every continent once hits have been restored, so the resurrected
	 * player doesn't return to a world still full of whatever was hunting
	 * them down before they died
	 */
	void removeCreaturesOnContinent(int continent);

	/**
	 * Picks a new random position for the player, on the same continent
	 * they died on, on a patch of grass or woods
	 */
	void resetPlayerPosition();

	/**
	 * Finishes the resurrection - loses any ridden transport and all
	 * weapons, unreadies everything, then returns to the overworld
	 */
	void finish();

public:
	Dead() : Interaction("Dead") {
	}
	~Dead() override {
	}

	bool msgFocus(const FocusMessage &msg) override;

	// The death sequence is automatic - swallow input rather than letting
	// it reach the (soon to be replaced) logic/view underneath
	bool msgAction(const ActionMessage &msg) override {
		return true;
	}
	bool msgKeypress(const KeypressMessage &msg) override {
		return true;
	}

	void draw() override;
	bool tick() override;
};

/**
 * Shown when the player dies in outer space (e.g. flying too close to a
 * star). Unlike the overworld's Dead, there's no automatic resurrection
 * sequence - the caller is expected to have already written the reason for
 * death to the commands area (via SpaceLogic::death()) before this view goes
 * up. Just shows the same skull glyph and waits for any key/action, then
 * returns to the main menu
 */
class SpaceDead : public Interaction {
public:
	SpaceDead() : Interaction("SpaceDead") {
	}
	~SpaceDead() override {
	}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	void draw() override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
