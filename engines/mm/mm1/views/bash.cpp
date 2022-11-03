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

#include "mm/mm1/views/bash.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {

Bash::Bash() : TextView("Bash") {
}

bool Bash::msgGame(const GameMessage &msg) {
	if (msg._name == "SHOW") {
		bashDoor();
		return true;
	}

	return false;
}

void Bash::bashDoor() {
	byte walls = g_maps->_currentWalls & g_maps->_forwardMask;
	if (!(g_maps->_currentState & 0x55 & g_maps->_forwardMask) || !walls) {
		forwards();
		return;
	}

	int val;
	if (!(walls & 0x55))
		val = 1;
	else if (!(g_maps->_currentWalls & g_maps->_currentWalls & 0xaa))
		val = 0;
	else
		val = 2;

	if (g_maps->_currentMap->dataByte(Maps::MAP_30 + val) != 1) {
		forwards();
		return;
	}

	Sound::sound(SOUND_1);
	int might = getRandomNumber(100);

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		might += c._might;
	}

	int threshold = g_maps->_currentMap->dataByte(Maps::MAP_45);

	if (threshold && might >= threshold) {
		g_maps->_currentMap->unlockDoor();
	}

	if (getRandomNumber(100) >= g_maps->_currentMap->dataByte(Maps::MAP_TRAP_THRESHOLD)) {
		send("Trap", GameMessage("TRIGGER"));
	} else {
		forwards();
	}
}

void Bash::forwards() {
	g_events->addAction(KEYBIND_FORWARDS);
}

} // namespace Views
} // namespace MM1
} // namespace MM
