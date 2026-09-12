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

#ifndef ULTIMA2_LOGIC_SPACE_COCKPIT_LOGIC_H
#define ULTIMA2_LOGIC_SPACE_COCKPIT_LOGIC_H

#include "ultima/ultima1/logic/space_logic.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

/**
 * The first-person cockpit view of outer space: a panning starscape where
 * the arrows steer the ship (Left/Right/Climb/Dive), the number keys set
 * the flight speed, and enemy-ship combat happens. Reached from the
 * overhead view with the View command
 */
class SpaceCockpitLogic : public SpaceLogic {
private:
	// tick() fires ~20/sec; only advance the starfield every few calls so
	// it doesn't streak by too fast
	int _tickCounter = 0;

	/**
	 * Sets the flight speed (1-8), spending 4 fuel per step of change
	 * (setSpeed) - the number keys
	 */
	void setSpeed(int speed);

	/**
	 * Per-frame update for the current encounter, if any (the target half
	 * of cockpitPerFrame): the target drifts a pixel further from the
	 * crosshair each frame unless the crosshair keeps tracking it, being
	 * dropped (setupSectorEnemies picks the next one) if it drifts out of
	 * the viewport; otherwise, on a speed-weighted random timer, advances
	 * how close it's approached (targetApproachStage), which widens the
	 * window fire() hit-tests shots against - past the last stage, it's
	 * also dropped
	 */
	void tickEncounter();

	/**
	 * Called from tickEncounter() for each frame a target is being engaged:
	 * a small chance of the target shooting back (alienFiresBack). Fires a
	 * laser-bolt line in from a random viewport edge toward the target, and
	 * a further chance of it actually hitting - draining shields (and
	 * killing the player outright if that empties them)
	 */
	void alienFiresBack();

protected:
	bool move(Data::Direction dir) override;

	/**
	 * The Fire command (handleFireCommand): spends fuel, flashes a laser
	 * bolt from each side of the viewport to the crosshair, and hit-tests
	 * it against the current encounter target, if any
	 */
	bool fire() override;
	bool pass() override;
	bool view() override;

	/**
	 * Validates fuel/heading and, if both are fine, hands off to the
	 * SpaceCockpit view's animated hyperjump sequence (hyperjump) - the
	 * actual sector change happens later, in completeHyperjump(), once that
	 * animation finishes
	 */
	bool hyperjump() override;

public:
	SpaceCockpitLogic() : SpaceLogic() {}
	~SpaceCockpitLogic() override {
	}

	void keypress(Common::KeyCode keycode) override;

	/**
	 * Advances the panning starfield each frame (the starfield half of
	 * cockpitPerFrame)
	 */
	void tick() override;

	/**
	 * Picks a fresh encounter target (a random on-screen position within
	 * the viewport) if this sector still has enemies remaining, or clears
	 * the current one if not (setupSectorEnemies). Called on first entering
	 * the cockpit view for a sector and again after each kill or hyperjump
	 * arrival
	 */
	void setupSectorEnemies();

	/**
	 * Called by the SpaceCockpit view once its hyperjump animation ramps up
	 * to full scrolling speed, to append " Lightspeed!" to the still-open
	 * "HyperJump..." line
	 */
	void hyperjumpLightspeed();

	/**
	 * Called by the SpaceCockpit view once its hyperjump animation
	 * completes - charges the fuel and actually moves the player's ship
	 * into a free slot in the sector one over in the given direction
	 * (dx/dy each -1/0/1), matching changeSector's own bookkeeping
	 */
	void completeHyperjump(int dx, int dy);

	/**
	 * Called by the SpaceCockpit view when a keypress/action interrupts an
	 * in-progress hyperjump animation - no fuel was spent yet, so this just
	 * reports it and lets the player carry on
	 */
	void abortHyperjump();
};

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima

#endif
