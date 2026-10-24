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

#include "mm/mm1/maps/map11.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"
#include "mm/mm1/utils/strings.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define MAX_ANSWER_LENGTH 8
#define ANSWER_OFFSET 636
#define VAL1 641
#define VAL2 642
#define VAL3 643

void Map11::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 14; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[65 + i]) {
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	g_globals->_treasure._items[2] = 0;

	if (getRandomNumber(50) == 10) {
		pit();

	} else {
		auto upperDial = [](byte c) -> byte {
			return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
		};

		// The dials are stored in VAL2 and VAL3. A copy of whether each is
		// correct is kept in the saved visit data, since the overlay data
		// is reloaded from disk when a save is loaded
		const byte dial1 = upperDial(_data[VAL2]);
		const byte dial2 = upperDial(_data[VAL3]);
		const bool rawStable = dial1 == 'B' && dial2 == 'J';
		const bool savedStable = (_visited[3 * MAP_W + 6] & 0x80) != 0 &&
			(_visited[3 * MAP_W + 8] & 0x80) != 0;

		// If the live overlay contains the correct settings, heal the
		// save-safe mirror as well. This makes old/partially patched visits
		// converge on one state without changing the save format.
		if (rawStable) {
			_visited[3 * MAP_W + 6] |= 0x80;
			_visited[3 * MAP_W + 8] |= 0x80;
		}

		if (rawStable || savedStable) {
			g_maps->_mapPos = Common::Point(7, 11);
		} else {
			g_maps->_mapPos.x = getRandomNumber(15);
			g_maps->_mapPos.y = getRandomNumber(15);
		}

		updateGame();
		send(SoundMessage("Poof! You've been teleported..."));
	}
}
void Map11::special00() {
	visitedExit();
	send(SoundMessage(
		"A ladder going up, take it (Y/N)?",
		[]() {
			g_maps->_mapPos = Common::Point(7, 2);
			g_maps->changeMapFromExit(0xf04, 2);
		}
	));
}

void Map11::special01() {
	if (_data[VAL1]) {
		// Teleport to outside the Inn of Sorpigal
		g_maps->_mapPos = Common::Point(8, 5);
		g_maps->changeMap(0x604, 1);
	} else {
		g_events->addView("VolcanoGod");
	}
}

void Map11::special02() {
	g_maps->clearSpecial();
	pit();
}

void Map11::special03() {
	send(SoundMessage(mapText("Painted in a black and white pattern,\n"
		"a sign reads: The second part is the\n"
		"most valuable.")));
}

void Map11::special04() {
	g_maps->clearSpecial();
	g_events->addView("VirginPrisoner");
}

void Map11::special05() {
	send(SoundMessage(mapText("This cavern is random until the dials\nare set.")));
}

void Map11::special06() {
	selectDial(0);
}

void Map11::special07() {
	selectDial(1);
}

void Map11::special08() {
	g_maps->clearSpecial();
	g_globals->_encounters.execute();
}

void Map11::pit() {
	if (g_globals->_activeSpells._s.levitate) {
		Common::String msg = "Jagged slime pit! Levitation saved you.";
		send(SoundMessage(msg));
		Sound::sound(SOUND_3);

	} else {
		reduceHP();
		reduceHP();
		send(SoundMessage("Jagged slime pit!"));
		Sound::sound(SOUND_3);
	}
}

void Map11::selectDial(int dialIndex) {
	_dialIndex = dialIndex;
	Common::String msg = Common::String::format(
		"Stabilization dial #%c set (A-Z)?", '1' + dialIndex);

	// Show the dial's current setting under the prompt
	byte current = _data[VAL2 + dialIndex];
	if (current >= 'a' && current <= 'z')
		current -= 'a' - 'A';
	Common::String currentLine = Common::String::format("Current: %c",
		(current >= 'A' && current <= 'Z') ? current : '-');

	send(SoundMessage(
		0, 1, msg,
		0, 2, currentLine,
		[](const Common::KeyState &ks) {
			if (ks.keycode >= Common::KEYCODE_a &&
					ks.keycode <= Common::KEYCODE_z) {
				Map11 &map = *static_cast<Map11 *>(g_maps->_currentMap);
				// Derive the selected letter from keycode, not KeyState::ascii.
				// This avoids backend/keymapper differences where ascii may be 0
				// or have unexpected case even though the A-Z keycode is valid.
				const char c = 'A' + (ks.keycode - Common::KEYCODE_a);
				g_events->close();
				map.setDialChar(c);
				map.none160();
			}
		}
	));
}

void Map11::challenge() {
	Game::Encounter &enc = g_globals->_encounters;

	enc.clearMonsters();
	enc.addMonster(10, 12);
	for (int i = 1; i < 10; ++i)
		enc.addMonster(7, 8);

	enc._levelIndex = 96;
	enc._manual = true;
	enc.execute();
}

void Map11::setDialChar(char c) {
	if (c >= 'a' && c <= 'z')
		c -= 'a' - 'A';

	_data[VAL2 + _dialIndex] = c;

	const int visitedOffset = (_dialIndex == 0) ?
		3 * MAP_W + 6 : 3 * MAP_W + 8;
	const char target = (_dialIndex == 0) ? 'B' : 'J';
	if (c == target)
		_visited[visitedOffset] |= 0x80;
	else
		_visited[visitedOffset] &= ~0x80;

	// Echo the chosen value below the original A-Z instruction so there is
	// unambiguous visual feedback about what the engine recorded.
	Common::String line1 = Common::String::format(
		"Stabilization dial #%c set (A-Z)?", '1' + _dialIndex);
	Common::String line2 = Common::String::format("Selected: %c", c);
	send(InfoMessage(0, 1, line1, 0, 2, line2));
}

void Map11::clue() {
	g_maps->_mapPos = Common::Point(0, 5);
	updateGame();
}

void Map11::riddleAnswer(const Common::String &answer) {
	Common::String properAnswer;
	for (int i = 0; i < 8 && _data[ANSWER_OFFSET + i]; ++i)
		properAnswer += _data[ANSWER_OFFSET + i] + 30;

	if (answer.equalsIgnoreCase(properAnswer)) {
		_data[VAL1]++;
		g_globals->_treasure._items[2] = KEY_CARD_ID;
		g_events->addAction(KEYBIND_SEARCH);
	} else {
		g_maps->_mapPos = Common::Point(7, 2);
		g_maps->changeMap(0xf04, 2);
	}
}

} // namespace Maps
} // namespace MM1
} // namespace MM
