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

#include "mm/mm1/views/locations/tavern.h"
#include "mm/mm1/data/text.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Locations {

Tavern::Tavern() : Location("Tavern") {
}

bool Tavern::msgFocus(const FocusMessage &msg) {
	send("View", GameMessage("LOCATION", LOC_TAVERN));
	g_globals->_currCharacter = &g_globals->_party[0];
	return true;
}

bool Tavern::msgKeypress(const KeypressMessage &msg) {
	// If timed message display, end the waiting
	if (endDelay())
		return true;

	switch (msg.keycode) {
	case Common::KEYCODE_a:
		haveADrink();
		break;
	case Common::KEYCODE_b:
		tipBartender();
		break;
	case Common::KEYCODE_c:
		listenForRumors();
		break;
	case Common::KEYCODE_g:
		g_globals->_currCharacter->gatherGold();
		redraw();
		break;
	case Common::KEYCODE_1:
	case Common::KEYCODE_2:
	case Common::KEYCODE_3:
	case Common::KEYCODE_4:
	case Common::KEYCODE_5:
	case Common::KEYCODE_6:
		changeCharacter(msg.keycode - Common::KEYCODE_1);
		break;
	default:
		break;
	}

	return true;
}

bool Tavern::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	if (msg._action == KEYBIND_ESCAPE) {
		leave();
		return true;
	}

	return false;
}

void Tavern::draw() {
	Location::draw();

	writeString(20, 1, "a) Have a drink\n");
	writeString(20, 2, "b) Tip bartender\n");
	writeString(20, 3, "c) Listen for rumors");
}

void Tavern::haveADrink() {
	if (g_globals->_currCharacter->_condition) {
		Sound::sound(SOUND_2);
		displayMessage("You look terrible, go see the clerics.");
	} else if (subtractGold(1)) {
		if (++g_globals->_currCharacter->_numDrinks < 3 ||
				g_engine->getRandomNumber(10) <
				g_globals->_currCharacter->_endurance) {
			displayMessage("Great stuff!");
		} else {
			if (!(g_globals->_currCharacter->_condition & BAD_CONDITION))
				g_globals->_currCharacter->_condition |= POISONED;

			Sound::sound(SOUND_2);
			displayMessage(13, "You feel sick!!");
		}
	}
}

void Tavern::tipBartender() {
	if (g_globals->_currCharacter->_condition) {
		displayMessage("You look terrible, go see the clerics.");
	} else if (subtractGold(1)) {
		if (g_globals->_currCharacter->_numDrinks == 0) {
			displayMessage("Have a drink, then we'll talk.");
		} else if (g_engine->getRandomNumber(3) != 3) {
			displayMessage("Thanks a lot, have another round!");
		} else {
			int townNum = g_maps->_currentMap->dataByte(Maps::MAP_ID);
			displayMessage(TAVERN_TIPS[townNum - 1][MIN(g_globals->_currCharacter->_numDrinks / 4, 3)]);
		}
	}
}

void Tavern::listenForRumors() {
	Common::String msg = "No rumors today.";
	if (!g_globals->_heardRumor) {
		g_globals->_heardRumor = true;
		msg = TAVERN_RUMORS[g_engine->getRandomNumber(16) - 1];
	}

	displayMessage(msg);
}

} // namespace Locations
} // namespace Views
} // namespace MM1
} // namespace MM
