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

#include "mm/mm1/views_enh/locations/tavern.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Locations {

Tavern::Tavern() : Location("Tavern", LOC_TAVERN) {
	_btnSprites.load("buy.icn");
	addButton(&_btnSprites, Common::Point(2, 45), 0, Common::KEYCODE_d, true);
	addButton(&_btnSprites, Common::Point(2, 56), 2, Common::KEYCODE_g, true);
	addButton(&_btnSprites, Common::Point(2, 67), 6, Common::KEYCODE_t, true);
	addButton(&_btnSprites, Common::Point(2, 78), 10, Common::KEYCODE_r, true);
	addButton(&_escSprite, Common::Point(24, 100), 0, KEYBIND_ESCAPE);
}

void Tavern::draw() {
	Location::draw();
	const Character &c = *g_globals->_currCharacter;

	setReduced(false);
	writeLine(0, STRING["enhdialogs.tavern.title"], ALIGN_MIDDLE);
	writeLine(1, STRING["enhdialogs.location.options_for"], ALIGN_MIDDLE);
	writeString(0, 23, c._name, ALIGN_MIDDLE);

	writeString(18, 46, STRING["enhdialogs.tavern.drink"]);
	writeString(18, 57, STRING["enhdialogs.tavern.gather"]);
	writeString(18, 68, STRING["enhdialogs.tavern.tip"]);
	writeString(18, 79, STRING["enhdialogs.tavern.rumor"]);

	writeLine(10, STRING["enhdialogs.location.gold"]);
	writeLine(10, Common::String::format("%d",
		g_globals->_currCharacter->_gold), ALIGN_RIGHT);

	setReduced(true);
	writeString(27, 122, STRING["enhdialogs.location.esc"]);
}

bool Tavern::msgGame(const GameMessage &msg) {
	Location::msgGame(msg);
/*
	if (msg._name == "UPDATE")
		checkCharacter();
		*/
	return true;
}

bool Tavern::msgKeypress(const KeypressMessage &msg) {
	// If a delay is active, end it
	if (endDelay())
		return true;

	switch (msg.keycode) {
	case Common::KEYCODE_d:
		haveADrink();
		break;
	case Common::KEYCODE_g:
		g_globals->_currCharacter->gatherGold();
		redraw();
		break;
	case Common::KEYCODE_t:
		tipBartender();
		break;
	case Common::KEYCODE_r:
		listenForRumors();
		break;
	default:
		return Location::msgKeypress(msg);
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

	return Location::msgAction(msg);
}

void Tavern::messageShown() {
	redraw();
}


void Tavern::haveADrink() {
	if (g_globals->_currCharacter->_condition) {
		Sound::sound(SOUND_2);
		displayMessage(STRING["dialogs.tavern.terrible"]);

	} else if (subtractGold(1)) {
		if (++g_globals->_currCharacter->_numDrinks < 3 ||
			g_engine->getRandomNumber(10) <
			g_globals->_currCharacter->_endurance) {
			displayMessage(STRING["dialogs.tavern.great_stuff"]);

		} else {
			if (!(g_globals->_currCharacter->_condition & BAD_CONDITION))
				g_globals->_currCharacter->_condition |= POISONED;

			Sound::sound(SOUND_2);
			displayMessage(STRING["dialogs.tavern.you_feel_sick"]);
		}
	}
}

void Tavern::tipBartender() {
	if (g_globals->_currCharacter->_condition) {
		displayMessage(STRING["dialogs.tavern.go_see_clerics"]);

	} else if (subtractGold(1)) {
		if (g_globals->_currCharacter->_numDrinks == 0) {
			displayMessage(STRING["dialogs.tavern.have_a_drink"]);

		} else if (g_engine->getRandomNumber(3) != 3) {
			displayMessage(STRING["dialogs.tavern.have_another_round"]);

		} else {
			int townNum = g_maps->_currentMap->dataByte(Maps::MAP_ID);
			displayMessage(STRING[Common::String::format(
				"dialogs.tavern.tips.%d_%d",
				townNum, g_globals->_currCharacter->_numDrinks
			)]);
		}
	}
}

void Tavern::listenForRumors() {
	Common::String msg = STRING["dialogs.tavern.rumors.none"];
	if (!g_globals->_heardRumor) {
		g_globals->_heardRumor = true;
		msg = STRING[Common::String::format(
			"dialogs.tavern.rumors.%d",
			g_engine->getRandomNumber(16))];
	}

	displayMessage(msg);
}

} // namespace Location
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
