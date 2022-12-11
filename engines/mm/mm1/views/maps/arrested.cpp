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

#include "mm/mm1/views/maps/arrested.h"
#include "mm/mm1/maps/map04.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Maps {

Arrested::Arrested() : TextView("Arrested") {
	_bounds = getLineBounds(20, 24);
}

bool Arrested::msgFocus(const FocusMessage &msg) {
	return TextView::msgFocus(msg);
}

void Arrested::draw() {
	clearSurface();
	Sound::sound(SOUND_2);

	writeString(0, 1, STRING["maps.map04.guards"]);
}

bool Arrested::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	switch (msg.keycode) {
	case Common::KEYCODE_a:
		attack();
		break;
	case Common::KEYCODE_b:
		bribe();
		break;
	case Common::KEYCODE_c:
		run();
		break;
	case Common::KEYCODE_d:
		surrender();
		break;
	default:
		break;
	}

	return true;
}

void Arrested::attack() {
	Game::Encounter &enc = g_globals->_encounters;
	int monsterCount = getRandomNumber(5);
	close();

	enc.clearMonsters();
	for (int i = 0; i < monsterCount; ++i)
		enc.addMonster(6, 10);

	enc._manual = true;
	enc._levelIndex = 64;
	enc._encounterType = Game::FORCE_SURPRISED;
	enc.execute();
}

void Arrested::bribe() {
	if (getRandomNumber(10) == 10) {
		// Success
		close();
	} else {
		// 8 year sentence for attempted bribery
		surrender(8);
	}
}

void Arrested::run() {
	close();
	g_globals->_currCharacter = &g_globals->_party[
		getRandomNumber(g_globals->_party.size()) - 1
	];
	g_globals->_currCharacter->_condition = ERADICATED;

	static_cast<MM1::Maps::Map04 *>(g_maps->_currentMap)->special08();
}

void Arrested::surrender(int numYears) {
	// Characters are aged by their prision sentence, and their gold halved
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];

		if ((int)c._age._base + numYears < 256)
			c._age._base += numYears;
		c._gold /= 2;
	}

	// Display sentence
	clearSurface();
	Sound::sound(SOUND_2);
	writeString(0, 1, STRING["maps.map04.sentence"]);
	writeNumber(numYears);

	(*g_maps->_currentMap)[MAP04_TREASURE_STOLEN] = 0;
	delaySeconds(3);
}

} // namespace Spells
} // namespace Views
} // namespace MM1
} // namespace MM
