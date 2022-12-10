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

#ifndef MM1_MAPS_MAP_DESERT_H
#define MM1_MAPS_MAP_DESERT_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class MapDesert : public Map {
protected:
	enum RandomMode { RND_BASIC, RND_FULL };
private:
	int _deadCountOffset;
	RandomMode _randomMode;

	/**
	 * Randomly turns the party left or right
	 */
	void lost();

protected:
	/**
	 * Handles the common part of special from desert maps
	 * dealing with being lost, and dying
	 */
	void desert();

public:
	/**
	 * Constructor
	 */
	MapDesert(uint index, const Common::String &name, uint16 id,
		byte defaultSection, int deadCountOffset, RandomMode rndMode) :
		Map(index, name, id, defaultSection),
		_deadCountOffset(deadCountOffset), _randomMode(rndMode) {}

	/**
	 * Allow mapping of the desert maps in the enhanced version
	 * if the party has the desert map item
	 */
	bool mappingAllowed() const override;
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
