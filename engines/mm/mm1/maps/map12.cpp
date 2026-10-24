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

#include "mm/mm1/maps/map12.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"
#include "mm/mm1/utils/strings.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define CORRECT 464
#define SELECTIONS 473

void Map12::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 18; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[69 + i]) {
				
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	// All other cells on the map are encounters
	g_maps->clearSpecial();
	g_globals->_encounters.execute();
}

void Map12::special00() {
	visitedExit();
	send(SoundMessage(
		"Ladder up, climb (Y/N)?",
		[]() {
			g_maps->_mapPos = Common::Point(7, 13);
			g_maps->changeMapFromExit(0x106, 2);
		}
	));
}

void Map12::special01() {
	Common::String line1 = "Platinum lever protrudes from the wall.";
	Common::String line2;

	for (int i = 0; i < 9; ++i) {
		if (!_data[SELECTIONS + i]) {
			line2 = "It won't budge!";
			break;
		}
	}

	if (!line2.empty()) {
		send(SoundMessage(0, 1, line1, 0, 2, line2));
	} else {
		line2 = "Pull it (Y/N)?";
		send(SoundMessage(
			0, 1, line1,
			0, 2, line2,
			[]() {
				Map12 &map = *static_cast<Map12 *>(g_maps->_currentMap);
				int i;
				for (i = 0; i < 9; ++i) {
					if (map[SELECTIONS + i] != map[CORRECT + i])
						break;
				}

				if (i < 9) {
					g_events->send(SoundMessage("Incorrect settings!"));

				} else {
					// Reward the party
					for (i = 0; i < (int)g_globals->_party.size(); ++i) {
						Character &c = g_globals->_party[i];
						if (c._intelligence._base < 25) {
							c._intelligence._current = c._intelligence._base =
								c._intelligence._base + 2;
						}

						c._gems = MIN((int)c._gems + 20, 0xffff);
						c._gold += 200;
						c._exp += 2000;
					}

					g_events->send(SoundMessage(mapText("You have mastered the magic square!\n"
						"+2 int, +20 gems, +200 gold, +2000 exp")));
				}
			}
		));
	}
}

void Map12::special02() {
	polyhedron('1', 0x80 + '6');
}

void Map12::special03() {
	polyhedron('1', 0x80 + '3');
}

void Map12::special04() {
	polyhedron('1', 0x80 + '0');
}

void Map12::special05() {
	polyhedron('1', '1');
}

void Map12::special06() {
	polyhedron('1', 0x80 + '2');
}

void Map12::special07() {
	polyhedron('1', 0x80 + '5');
}

void Map12::special08() {
	polyhedron('1', 0x80 + '4');
}

void Map12::special09() {
	setPolyhedron(0);
}

void Map12::special10() {
	setPolyhedron(1);
}

void Map12::special11() {
	setPolyhedron(2);
}

void Map12::special12() {
	setPolyhedron(3);
}

void Map12::special13() {
	setPolyhedron(4);
}

void Map12::special14() {
	setPolyhedron(5);
}

void Map12::special15() {
	setPolyhedron(6);
}

void Map12::special16() {
	setPolyhedron(7);
}

void Map12::special17() {
	setPolyhedron(8);
}

void Map12::polyhedron(unsigned char side1, unsigned char side2) {
	Common::String msg = Common::String::format(
		"A glowing polyhedron is set on side %c%c", side1, side2);
	send(SoundMessage(msg));
}

void Map12::keyCallbackSpinPolyhedronTwo() {
	static_cast<Map12 *>(g_maps->_currentMap)->spinPolyhedron(0);
	g_maps->_currentMap->updateGame();
}

void Map12::keyCallbackSpinPolyhedronOne(const Common::KeyState &ks) {
	if (ks.keycode >= Common::KEYCODE_0 && ks.keycode <= Common::KEYCODE_9) {
		g_events->close();
		Map12 &map = *static_cast<Map12 *>(g_maps->_currentMap);
		map.spinPolyhedron(ks.ascii | 0x80);
		map.none160();
	}
}

void Map12::setPolyhedron(int polyIndex) {
	_polyIndex = polyIndex;

	if (_data[SELECTIONS + polyIndex]) {
		Common::String msg = Common::String::format(
			mapText("The polyhedron is set on %c\nSpin it (Y/N)?").c_str(),
			_data[SELECTIONS + polyIndex]);
		send(SoundMessage(msg, keyCallbackSpinPolyhedronTwo));
	} else {
		send(SoundMessage(mapText("Atop a crystal pyramid spins a glowing\n"
			"polyhedron. Stop it on side (1-9)?"), keyCallbackSpinPolyhedronOne));
	}
}

void Map12::spinPolyhedron(byte newSide) {
	_data[SELECTIONS + _polyIndex] = newSide;
}

} // namespace Maps
} // namespace MM1
} // namespace MM
