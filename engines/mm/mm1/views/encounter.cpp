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

#include "mm/mm1/views/encounter.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

Encounter::Encounter() : TextView("Encounter") {
}

void Encounter::draw() {
	const Game::Encounter &enc = g_globals->_encounters;

	// Clear the commands area
	Graphics::ManagedSurface s = getSurface();
	s.fillRect(Common::Rect(31 * 8, 0, 320, 17 * 8), 0);

	// Write the monster list
	for (uint i = 0; i < enc._monsterList.size(); ++i) {
		writeChar(22, i, 'A' + i);
		writeString(") ");
		writeString(enc._monsterList[i]._name);
	}

	// Display the monster
	drawMonster(enc._val8);

	// Write the encounter options

}

void Encounter::drawMonster(int monsterNum) {
	Graphics::ManagedSurface img =
		g_globals->_monsters.getMonsterImage(monsterNum);
	getSurface().blitFrom(img, Common::Point(20, 20));
}

} // namespace Views
} // namespace MM1
} // namespace MM
