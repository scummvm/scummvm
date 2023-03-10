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
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

Unlock::Unlock() : PartyView("Unlock") {
	_bounds = Common::Rect(234, 144, 320, 200);
}

bool Unlock::msgGame(const GameMessage &msg) {
	if (msg._name == "SHOW") {
		byte walls = g_maps->_currentWalls & g_maps->_forwardMask;

		if (!(g_maps->_currentState & 0x55 & g_maps->_forwardMask) || !walls) {
			g_globals->_party.checkPartyDead();
			return true;
		}

		int offset;
		if (!(walls & 0x55)) {
			offset = 1;
		} else if (!(walls & 0xaa)) {
			offset = 0;
		} else {
			offset = 2;
		}

		if (g_maps->_currentMap->dataByte(30 + offset) != 1) {
			g_globals->_party.checkPartyDead();
			return true;
		}

		if (g_globals->_party.size() > 1) {
			// Select the character to use
			open();
		} else {
			// With only one party member, they're automatically used
			charSelected(0);
		}
	} else if (msg._name == "UPDATE") {
		charSelected(g_globals->_party.indexOf(g_globals->_currCharacter));
	}

	return true;
}

void Unlock::draw() {
	PartyView::draw();

	writeString(Common::String::format(
		STRING["dialogs.unlock.who_will_try"].c_str(),
		'0' + g_globals->_party.size()));
}


bool Unlock::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		return true;
	} else if (msg._action >= KEYBIND_VIEW_PARTY1 &&
		msg._action <= KEYBIND_VIEW_PARTY6) {
		charSelected(msg._action - KEYBIND_VIEW_PARTY1);
		return true;
	}

	return PartyView::msgAction(msg);
}

void Unlock::charSelected(uint charIndex) {
	if (charIndex >= g_globals->_party.size())
		return;

	Character &c = g_globals->_party[charIndex];
	g_globals->_currCharacter = &c;

	if (c._condition & (BAD_CONDITION | DEAD | STONE | ASLEEP)) {
		Sound::sound(SOUND_2);
		draw();

	} else {
		if (isFocused())
			close();

		int val = g_maps->_currentMap->dataByte(Maps::MAP_49) * 4 +
			getRandomNumber(100);

		if (val < c._trapCtr) {
			g_maps->_currentMap->unlockDoor();
			send(InfoMessage(11, 1, STRING["dialogs.unlock.success"]));

		} else if (getRandomNumber(100) <
				g_maps->_currentMap->dataByte(Maps::MAP_TRAP_THRESHOLD)) {
			send(InfoMessage(8, 1, STRING["dialogs.unlock.failed"]));

		} else {
			g_maps->_currentMap->unlockDoor();
			send("Trap", GameMessage("TRIGGER"));
		}
	}
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
