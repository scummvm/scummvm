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

#ifndef MM1_MAPS_MAP_TOWN_H
#define MM1_MAPS_MAP_TOWN_H

#include "mm/mm1/maps/map.h"

namespace MM {
namespace MM1 {
namespace Maps {

class MapTown : public Map {
protected:
	/**
	 * Handles visiting blacksmith
	 */
	void blacksmith();

	/**
	 * Handles visiting inn
	 */
	void inn();

	/**
	 * Handles visiting market
	 */
	void market();

	/**
	 * Handles visiting tavern
	 */
	void tavern();

	/**
	 * Handles visiting temple
	 */
	void temple();

	/**
	 * Handles visiting trainer
	 */
	void training();

	/**
	 * Show a sign message
	 */
	void showSign(const Common::String &msg);
public:
	MapTown(uint index, const Common::String &name, uint16 id,
		byte defaultSection, const char *desc = nullptr) :
		Map(index, name, id, defaultSection, desc) {}
	virtual ~MapTown() {}
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
