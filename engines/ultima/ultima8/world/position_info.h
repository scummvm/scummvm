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

#ifndef ULTIMA8_WORLD_POSITION_INFO_H
#define ULTIMA8_WORLD_POSITION_INFO_H

namespace Ultima {
namespace Ultima8 {

class Item;

/**
 * Position information for a desired target box for an item.
 * The position is valid when the target box does not collide with any solid items.
 *
 * Land is set to the item with the highest z coordinate under the target box,
 * or null if there is no land below box.
 * 
 * Roof is set to the item with the lowest z coordinate over the target box,
 * or null if there is no roof above box.
 *
 * Blocker is set to an item blocking the target box, or null if there is no such item.
 *
 * Supported is true if the land is appropriate to support the item when at the target.
 */
struct PositionInfo {
	bool valid;
	bool supported;
	const Item *land;
	const Item *roof;
	const Item *blocker;

	PositionInfo() : valid(false), supported(false), land(nullptr), roof(nullptr), blocker(nullptr) {}
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
