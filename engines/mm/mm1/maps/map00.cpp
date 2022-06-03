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

#include "mm/mm1/maps/map00.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Maps {

void Map00::special() {
	Maps &maps = g_globals->_maps;

	// Scan for special actions on the map cell
	for (uint i = 0; i < _data[50]; ++i) {
		if (maps._mapOffset == _data[51 + i] &&
				(maps._forwardMask & _data[75 + i]) != 0) {
			// TODO: Some extra message code
			(this->*SPECIAL_FN[i])();
			return;
		}
	}

	// All other cells on the map are encounters
	maps.clearSpecial();
	g_events->addView("Encounter");
}

void Map00::special00() {}
void Map00::special01() {}
void Map00::special02() {}
void Map00::special03() {}
void Map00::special04() {}
void Map00::special05() {}
void Map00::special06() {}
void Map00::special07() {}
void Map00::special08() {}
void Map00::special09() {}
void Map00::special10() {}
void Map00::special11() {}
void Map00::special12() {}
void Map00::special13() {}
void Map00::special14() {}
void Map00::special15() {}
void Map00::special16() {}
void Map00::special17() {}
void Map00::special18() {}
void Map00::special19() {}
void Map00::special20() {}
void Map00::special21() {}
void Map00::special22() {}
void Map00::special23() {}

} // namespace Maps
} // namespace MM1
} // namespace MM
