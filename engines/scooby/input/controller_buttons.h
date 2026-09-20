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

#ifndef SCOOBY_CONTROLLER_BUTTONS_H
#define SCOOBY_CONTROLLER_BUTTONS_H

#include "common/scummsys.h"

// Identifies the held directional and action inputs consumed by the
// recovered input logic. [Flags] enum in the source -> enum class plus
// bitwise operator overloads (C++11 enum class has no built-in bitwise ops).

namespace Scooby {

enum class ControllerButtons : uint8 {
	None = 0,
	Up = 1 << 0,    // Move Shaggy or the cursor upward.
	Down = 1 << 1,  // Move Shaggy or the cursor downward.
	Left = 1 << 2,  // Move Shaggy or the cursor left.
	Right = 1 << 3, // Move Shaggy or the cursor right.
	B = 1 << 4,     // Activate the cursor, command window, inventory, or selected interaction.
	C = 1 << 5,     // Toggle between the command window and inventory, or leave the sound-test menu.
	A = 1 << 6,     // Cancel the current action and return to controlling Shaggy.
	Start = 1 << 7  // Open the current password display.
};

inline ControllerButtons buttonMask(ControllerButtons a) { return static_cast<ControllerButtons>(0xff ^ static_cast<uint8>(a)); }

inline ControllerButtons operator|(ControllerButtons a, ControllerButtons b) {
	return static_cast<ControllerButtons>(static_cast<uint8>(a) | static_cast<uint8>(b));
}

inline ControllerButtons operator&(ControllerButtons a, ControllerButtons b) {
	return static_cast<ControllerButtons>(static_cast<uint8>(a) & static_cast<uint8>(b));
}

inline ControllerButtons operator^(ControllerButtons a, ControllerButtons b) {
	return static_cast<ControllerButtons>(static_cast<uint8>(a) ^ static_cast<uint8>(b));
}

inline ControllerButtons operator~(ControllerButtons a) {
	return static_cast<ControllerButtons>(static_cast<uint8>(~static_cast<uint8>(a)));
}

inline ControllerButtons &operator|=(ControllerButtons &a, ControllerButtons b) { return a = a | b; }

inline ControllerButtons &operator&=(ControllerButtons &a, ControllerButtons b) { return a = a & b; }

inline bool Any(ControllerButtons value) { return static_cast<uint8>(value) != 0; }
} // namespace Scooby

#endif // SCOOBY_CONTROLLER_BUTTONS_H
