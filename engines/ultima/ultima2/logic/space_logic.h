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


#ifndef ULTIMA2_LOGIC_SPACE_LOGIC_H
#define ULTIMA2_LOGIC_SPACE_LOGIC_H

#include "ultima/ultima2/logic/logic.h"

namespace Ultima {
namespace Ultima2 {
namespace Logic {

/**
 * Command handling while flying a rocket between worlds. There's no turn
 * structure: the ship can be steered, hyperwarped to a set of XENO/YAKO/ZABO
 * coordinates, and landed when orbiting a world
 */
class SpaceLogic : public Logic {
private:
	// The coordinates being flown to, or that were last reached
	byte _coords[3] = {};

	// Set until the first hyperwarp after launch completes
	bool _launching = false;

	/**
	 * Reports the world the coordinates put the ship at. Returns false if
	 * that was fatal
	 */
	bool announceOrbit();

	/**
	 * Starts a hyperwarp, once fuel is confirmed
	 */
	void hyperwarp();

	/**
	 * Lands on the world being orbited
	 */
	void land();

public:
	~SpaceLogic() override {}

	/**
	 * Takes off from the current world, switching to space flight
	 */
	static void takeOff();

	const byte *coords() const {
		return _coords;
	}

	/**
	 * Called with the coordinates the player chose to hyperwarp to
	 */
	void hyperwarpTo(int xeno, int yako, int zabo);

	/**
	 * Called by the space view once a hyperwarp animation completes
	 */
	void warpFinished();

	void action(int action) override;
	void keypress(const Shared::Messages::KeypressMessage &msg) override;
};

} // namespace Logic
} // namespace Ultima2
} // namespace Ultima

#endif
