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

#ifndef ULTIMA2_LOGIC_SPACE_MAP_LOGIC_H
#define ULTIMA2_LOGIC_SPACE_MAP_LOGIC_H

#include "ultima/ultima1/logic/space_logic.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

/**
 * The overhead sector-map view of outer space: the arrows nudge the ship's
 * drift heading (West/East/North/South - handleOverheadArrows), and the
 * drift itself is applied each frame (per_frame_logic), clamped to the
 * sector's playable area. Flying into the viewport border bounces the ship
 * back and drains shields ("Crunch!" - handleSectorBoundaryCollision);
 * actually crossing into a neighbouring sector is a cockpit-only feature
 * (hyperjump), not available from this view. This is the mode take-off
 * loads into
 */
class SpaceMapLogic : public SpaceLogic {
protected:
	bool move(Data::Direction dir) override;
	bool fire() override;
	bool view() override;

	/**
	 * Repurposed as an "Autopilot" testing shortcut in the overhead view -
	 * finds a free station docking port and instantly shifts the ship
	 * there, then triggers docking directly (not part of the original)
	 */
	bool attack(Data::Direction dir) override;

	/**
	 * Hyperjump can only be done from the cockpit view (hyperjump)
	 */
	bool hyperjump() override;

public:
	SpaceMapLogic() : SpaceLogic() {}
	~SpaceMapLogic() override {
	}

	/**
	 * Applies the current drift to the player's ship position, handling the
	 * viewport-border bounce/shield-drain and advancing the exhaust trail
	 * countdown (per_frame_logic, overhead branch)
	 */
	void tick() override;
};

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima

#endif
