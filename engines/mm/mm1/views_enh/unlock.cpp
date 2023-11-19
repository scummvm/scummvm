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

#include "mm/mm1/views_enh/unlock.h"
#include "mm/mm1/views_enh/who_will_try.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

Unlock::Unlock() : PartyView("Unlock") {
	_bounds = Common::Rect(234, 144, 320, 200);
}

bool Unlock::msgGame(const GameMessage &msg) {
	if (msg._name != "SHOW")
		return false;

	byte walls = g_maps->_currentWalls & g_maps->_forwardMask;

	bool canUnlock = true;
	if (!(g_maps->_currentState & 0x55 & g_maps->_forwardMask) || !walls)
		canUnlock = false;

	int offset;
	if (!(walls & 0x55)) {
		offset = 1;
	} else if (!(walls & 0xaa)) {
		offset = 0;
	} else {
		offset = 2;
	}

	if (g_maps->_currentMap->dataByte(30 + offset) != 1)
		canUnlock = false;

	if (canUnlock) {
		WhoWillTry::display(charSelected);
	} else {
		g_events->send(InfoMessage(STRING["dialogs.unlock.none"]));
	}

	return true;
}

void Unlock::charSelected(int charIndex) {
	if (charIndex == -1 || charIndex >= (int)g_globals->_party.size())
		return;

	Character &c = g_globals->_party[charIndex];
	g_globals->_currCharacter = &c;

	if (c._condition & (BAD_CONDITION | DEAD | STONE | ASLEEP)) {
		Sound::sound(SOUND_2);

	} else {
		int val = g_maps->_currentMap->dataByte(Maps::MAP_49) * 4 +
			g_engine->getRandomNumber(100);

		if (val < c._trapCtr) {
			g_maps->_currentMap->unlockDoor();
			g_engine->send(InfoMessage(11, 1, STRING["dialogs.unlock.success"]));

		} else if (g_engine->getRandomNumber(100) <
				g_maps->_currentMap->dataByte(Maps::MAP_TRAP_THRESHOLD)) {
			g_engine->send(InfoMessage(8, 1, STRING["dialogs.unlock.failed"]));

		} else {
			g_maps->_currentMap->unlockDoor();
			g_engine->send("Trap", GameMessage("TRIGGER"));
		}
	}
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
