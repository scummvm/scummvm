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

#include "mm/mm1/views/search.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {

Search::Search() : TextView("Search") {
	_bounds = getLineBounds(20, 24);
}

bool Search::msgGame(const GameMessage &msg) {
	if (msg._name != "SHOW")
		return false;

	bool hasStuff = g_globals->_treasure.present();
	if (hasStuff) {
		// Focus view to show what was found
		open();

	} else {
		// Otherwise send an info message to say nothing was found
		Common::String line = STRING["dialogs.search.search"] +
			STRING["dialogs.search.nothing"];
		send(InfoMessage(0, 1, line));
	}

	return true;
}

bool Search::msgFocus(const FocusMessage &msg) {
	_bounds = getLineBounds(20, 24);
	_lineNum = 0;

	if (_mode == FOCUS_GET_TREASURE) {
		// Returning from trap display
		if (g_globals->_party.checkPartyDead())
			return true;

		getTreasure();
	} else {
		_mode = INITIAL;
	}

	return true;
}

void Search::draw() {
	Common::String line;
	if (_mode != GET_ITEMS)
		clearSurface();

	switch (_mode) {
	case INITIAL:
		Sound::sound(SOUND_2);
		line = STRING["dialogs.search.search"] +
			STRING["dialogs.search.you_found"];
		writeString(0, 1, line);
		delaySeconds(2);
		break;

	case OPTIONS:
		writeString(1, 1, STRING["dialogs.search.options"]);
		writeString(20, 1, STRING["dialogs.search.options1"]);
		writeString(20, 2, STRING["dialogs.search.options2"]);
		writeString(20, 3, STRING["dialogs.search.options3"]);
		escToGoBack(0, 3);
		break;

	case GET_TREASURE:
		drawTreasure();
		break;

	case GET_ITEMS:
		// This may be called up to three times, for each item
		drawItem();
		break;

	case WHO_WILL_TRY: {
		line = Common::String::format(
			STRING["dialogs.misc.who_will_try"].c_str(),
			'0' + g_globals->_party.size()
		);
		writeString(10, 1, line);
		break;
	}

	default:
		break;
	}
}

bool Search::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	switch (_mode) {
	case INITIAL:
		endDelay();
		break;

	case OPTIONS:
		switch (msg.keycode) {
		case Common::KEYCODE_1:
			openContainer();
			break;
		case Common::KEYCODE_2:
			findRemoveTrap();
			break;
		case Common::KEYCODE_3:
			detectMagicTrap();
			break;
		default:
			break;
		}
		break;

	case RESPONSE:
		endDelay();
		break;

	case WHO_WILL_TRY:
		if (msg.keycode >= Common::KEYCODE_1 &&
			msg.keycode <= (Common::KEYCODE_0 + (int)g_globals->_party.size())) {
			// Character selected
			g_globals->_currCharacter = &g_globals->_party[
				msg.keycode - Common::KEYCODE_1];
			if ((g_globals->_currCharacter->_condition &
				(BAD_CONDITION | DEAD | STONE | ASLEEP)) != 0) {
				clearSurface();
				writeString(3, 2, STRING["dialogs.search.check_condition"]);
				delaySeconds(4);
			} else if (_removing) {
				findRemoveTrap2();
			} else {
				openContainer2();
			}
		}
		break;

	case GET_TREASURE:
		break;

	default:
		break;
	}

	return true;
}

bool Search::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	if (msg._action == KEYBIND_ESCAPE) {
		switch (_mode) {
		case OPTIONS:
			close();
			break;
		case WHO_WILL_TRY:
			_mode = OPTIONS;
			draw();
			break;
		default:
			break;
		}

		return true;
	}

	return true;
}

void Search::timeout() {
	switch (_mode) {
	case INITIAL: {
		Maps::Map &map = *g_maps->_currentMap;
		_val1 = MIN(g_globals->_treasure._container * 8 +
			map[Maps::MAP_TRAP_THRESHOLD], 255);

		if (!g_globals->_treasure._trapType) {
			g_globals->_treasure._trapType = (getRandomNumber(100) < _val1) ? 1 : 2;
		}

		// Show the name of the container type in the game view
		send("View", HeaderMessage(
			STRING[Common::String::format("dialogs.search.containers.%d",
				g_globals->_treasure._container)]
		));

		// Display a graphic for the container type
		int gfxNum = g_globals->_treasure._container < WOODEN_BOX ? 4 : 2;
		send("View", DrawGraphicMessage(gfxNum + 65));

		_mode = OPTIONS;
		draw();
		break;
	}
	case RESPONSE:
		_mode = OPTIONS;
		draw();
		break;

	case WHO_WILL_TRY:
		draw();
		break;

	case GET_TREASURE:
		_mode = GET_ITEMS;
		draw();
		break;

	case GET_ITEMS:
		draw();
		break;

	case GET_ITEMS_DONE:
		close();
		break;

	default:
		break;
	}
}

void Search::openContainer() {
	_removing = false;
	if (whoWillTry())
		openContainer2();
}

void Search::openContainer2() {
	if (g_globals->_treasure._trapType == 1) {
		Maps::Map &map = *g_maps->_currentMap;
		int thresold = map[Maps::MAP_TRAP_THRESHOLD] +
			g_globals->_treasure._container;

		if (getRandomNumber(thresold + 5) < thresold) {
			// Triggered a trap
			_mode = FOCUS_GET_TREASURE;
			g_events->send("Trap", GameMessage("TRAP"));
			return;
		}
	}

	getTreasure();
}

void Search::findRemoveTrap() {
	_removing = true;
	if (whoWillTry())
		findRemoveTrap2();
}

void Search::findRemoveTrap2() {
	Character &c = *g_globals->_currCharacter;

	if (g_globals->_treasure._trapType == 1) {
		byte val = c._trapCtr;
		if (getRandomNumber(val >= 100 ? val + 5 : 100) >= val) {
			// Triggered a trap
			g_events->send("Trap", GameMessage("TRAP"));
			return;
		}
	}
}

void Search::detectMagicTrap() {
	Character &c = *g_globals->_currCharacter;
	_mode = RESPONSE;

	if (c._class == PALADIN || c._class == CLERIC) {
		Sound::sound(SOUND_2);
		clearSurface();
		writeString(6, 2, STRING["dialogs.search.bad_class"]);
		delaySeconds(4);

	} else if (c._sp == 0) {
		Sound::sound(SOUND_2);
		clearSurface();
		writeString(6, 2, STRING["dialogs.search.no_sp"]);
		delaySeconds(4);

	} else {
		c._sp._current--;
		char magic = g_globals->_treasure.hasItems() ||
			g_globals->_treasure.getGems() ? 'Y' : 'N';
		char trapped = g_globals->_treasure._trapType == 1 ? 'Y' : 'N';

		clearSurface();
		writeString(5, 1, Common::String::format(
			STRING["dialogs.search.magic_trap"].c_str(),
			magic, trapped));

		delaySeconds(8);
	}
}

bool Search::whoWillTry() {
	if (g_globals->_party.size() == 1) {
		g_globals->_currCharacter = &g_globals->_party[0];
		return true;
	} else {
		// Switch to mode to ask which character to use
		_mode = WHO_WILL_TRY;
		draw();
		return false;
	}
}

void Search::getTreasure() {
	_mode = GET_TREASURE;
	_bounds = getLineBounds(17, 24);

	// Display a graphic for the container type
	int gfxNum = g_globals->_treasure._container < WOODEN_BOX ? 3 : 1;
	send("View", DrawGraphicMessage(gfxNum + 65));

	redraw();
}

void Search::drawTreasure() {
	writeString(15, 0, STRING["dialogs.search.it_opens"]);

	// Split up the gold across the party
	uint32 goldPerPerson = g_globals->_treasure.getGold() /
		g_globals->_party.size();
	g_globals->_treasure.setGold(0);

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		uint32 newGold = c._gold + goldPerPerson;
		if (newGold < c._gold)
			// As unlikely as it is to overflow 32-bits
			newGold = 0xffffffff;
		c._gold = newGold;
	}
	
	writeString(0, 2, Common::String::format(
		STRING["dialogs.search.each_share"].c_str(),
		goldPerPerson));
	g_globals->_treasure.setGold(0);

	// Assign any gems to a random person
	int gems = g_globals->_treasure.getGems();
	g_globals->_treasure.setGems(0);

	_lineNum = 3;
	if (gems) {
		// Choose a random recipient
		uint charNum = getRandomNumber(g_globals->_party.size()) - 1;
		Character &c = g_globals->_party[charNum];

		writeString(0, _lineNum++, Common::String::format(
			STRING["dialogs.search.found_gems"].c_str(),
			c._name,
			gems));
		c._gems = MIN((int)c._gems + gems, 0xffff);
	}

	Sound::sound2(SOUND_5);
	delaySeconds(2);
}

void Search::drawItem() {
	Treasure &treasure = g_globals->_treasure;
	int itemId = treasure.removeItem();

	// Iterate through any treasure items
	if (itemId != 0) {
		// Find a person with free backpack space for the item
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];

			// Check if character has backpack space
			if (c._backpack.full())
				continue;

			Item *item = g_globals->_items.getItem(itemId);
			c._backpack.add(itemId, item->_maxCharges);

			// Add line for found item
			writeString(0, _lineNum++, Common::String::format(
				STRING["dialogs.search.found_item"].c_str(),
				c._name,
				item->_name.c_str()
			));

			delaySeconds(2);
			return;
		}
	}

	// At this point we've either displayed the up to 3 item
	// lines (in addition to gold and/or gems), or the party's
	// backpacks were completely full up. Wait for 7 seconds
	_mode = GET_ITEMS_DONE;
	delaySeconds(7);
}

} // namespace Views
} // namespace MM1
} // namespace MM
