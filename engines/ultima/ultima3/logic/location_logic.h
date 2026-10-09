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

#ifndef ULTIMA3_LOGIC_LOCATION_LOGIC_H
#define ULTIMA3_LOGIC_LOCATION_LOGIC_H

#include "ultima/ultima3/logic/logic.h"

namespace Ultima {
namespace Ultima3 {

constexpr byte TRANSPORT_HORSE = 0x0A;
constexpr byte TRANSPORT_SHIP = 0x0B;
constexpr byte TRANSPORT_ON_FOOT = 0x3F;

/**
 * Rules for moving the party around a map
 */
class LocationLogic : public Logic {
private:
	/**
	 * Returns true if a ship can't sail in a direction because the wind is
	 * absent or blowing against it
	 */
	bool isShipBlockedByWind(Direction dir) const;

	/**
	 * Returns true if the party can't step onto terrain. Some terrain
	 * doesn't stop them but hurts, and some hurts and does stop them
	 */
	bool isTerrainBlocked(byte tile);

public:
	enum ExitResult {
		EXIT_NOT_RIDING,
		EXIT_NOT_HERE,
		EXIT_DONE
	};

	/**
	 * Moves the party a step in a direction, if they can
	 * @returns		True if they moved
	 */
	bool move(Direction dir);

	/**
	 * Mounts a horse or boards a ship that the party is standing on
	 * @returns		What to say about it, or null if there's nothing to board
	 */
	const char *board();

	/**
	 * Gets off a horse or ship, leaving it behind
	 */
	ExitResult exitVehicle();
};

} // namespace Ultima3
} // namespace Ultima

#endif
