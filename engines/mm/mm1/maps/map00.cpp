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
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

void Map00::special() {
	Maps &maps = g_globals->_maps;

	// Scan for special actions on the map cell
	for (uint i = 0; i < _data[50]; ++i) {
		if (maps._mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (maps._forwardMask & _data[75 + i]) {
				// TODO: Some extra message code
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	// All other cells on the map are encounters
	maps.clearSpecial();
	g_events->addView("Encounter");
}

void Map00::special00() { warning("special00"); }

void Map00::special01() {
	Maps &maps = g_globals->_maps;
	Sound::sound(SOUND_2);

	Common::String line2;
	int x = 5;

	switch (maps._forwardMask) {
	case DIRMASK_E:
		line2 = STRING["maps.map00.eulards"];
		x = 6;
		break;
	case DIRMASK_W:
		line2 = STRING["maps.map00.blacksmith"];
		break;
	default:
		line2 = STRING["maps.map00.inn"];
		break;
	}

	g_events->msgInfo(InfoMessage(
		2, 0, STRING["maps.map00.sign"],
		x, 1, line2
	));
}

void Map00::special02() { warning("special02"); }
void Map00::special03() { warning("special03"); }
void Map00::special04() { warning("special04"); }
void Map00::special05() { warning("special05"); }
void Map00::special06() { warning("special06"); }
void Map00::special07() { warning("special07"); }
void Map00::special08() { warning("special08"); }
void Map00::special09() { warning("special09"); }
void Map00::special10() { warning("special10"); }
void Map00::special11() { warning("special11"); }
void Map00::special12() { warning("special12"); }
void Map00::special13() { warning("special13"); }
void Map00::special14() { warning("special14"); }
void Map00::special15() { warning("special15"); }
void Map00::special16() { warning("special16"); }
void Map00::special17() { warning("special17"); }
void Map00::special18() { warning("special18"); }
void Map00::special19() { warning("special19"); }
void Map00::special20() { warning("special20"); }
void Map00::special21() { warning("special21"); }
void Map00::special22() { warning("special22"); }
void Map00::special23() { warning("special23"); }

} // namespace Maps
} // namespace MM1
} // namespace MM
