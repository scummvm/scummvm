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

#include "mm/mm1/views_enh/search.h"
#include "mm/mm1/views_enh/select_number.h"
#include "mm/mm1/views_enh/who_will_try.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

Search::Search() : SelectNumber("Search") {
	_bounds = Common::Rect(0, 144, 234, 200);
	_escSprite.load("esc.icn");
	addButton(&_escSprite, Common::Point(79, 30), 0, KEYBIND_ESCAPE, true);
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
	_lineNum = 0;
	_bounds = Common::Rect(0, 144, 234, 200);

	if (_mode == FOCUS_GET_TREASURE) {
		// Returning from trap display
		if (g_globals->_party.checkPartyDead())
			return true;

		getTreasure();
	} else {
		setMode(INITIAL);
	}

	return true;
}

void Search::draw() {
	Common::String line;
	setButtonEnabled(0, _mode == OPTIONS);

	//if (_mode != GET_ITEMS)
	SelectNumber::draw();

	switch (_mode) {
	case INITIAL:
		Sound::sound(SOUND_2);
		line = STRING["dialogs.search.search"] +
			STRING["dialogs.search.you_found"];
		writeString(0, 0, line);
		delaySeconds(2);
		break;

	case OPTIONS:
		writeString(0, 0, STRING["dialogs.search.options"]);
		writeString(80, 0, STRING["dialogs.search.options1"]);
		writeString(80, 9, STRING["dialogs.search.options2"]);
		writeString(80, 18, STRING["dialogs.search.options3"]);
		writeString(96, 32, STRING["enhdialogs.misc.go_back"]);
		break;

	case GET_TREASURE:
		drawTreasure();
		break;

	case GET_ITEMS:
		// This may be called up to three times, for each item
		drawItem();
		break;

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
			closeNumbers();
			openContainer();
			break;
		case Common::KEYCODE_2:
			closeNumbers();
			findRemoveTrap();
			break;
		case Common::KEYCODE_3:
			closeNumbers();
			detectMagicTrap();
			break;
		default:
			break;
		}
		break;

	case RESPONSE:
		endDelay();
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

		setMode(OPTIONS);
		draw();
		break;
	}
	case RESPONSE:
		setMode(OPTIONS);
		draw();
		break;

	case GET_TREASURE:
		setMode(GET_ITEMS);
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
			setMode(FOCUS_GET_TREASURE);
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
	setMode(RESPONSE);

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
		WhoWillTry::display([](int charNum) {
			static_cast<Search *>(g_events->findView("Search"))->whoWillTry(charNum);
		});
		return false;
	}
}

void Search::whoWillTry(int charNum) {
	if (charNum == -1) {
		// Character selection aborted, go back to options
		setMode(OPTIONS);

	} else {
		// Character selected, proceed with given action
		g_globals->_currCharacter = &g_globals->_party[charNum];

		if (_removing) {
			findRemoveTrap2();
		} else {
			openContainer2();
		}
	}

	open();
}

void Search::getTreasure() {
	setMode(GET_TREASURE);
	_bounds = Common::Rect(0, 144, 234, 200);

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
	setMode(GET_ITEMS_DONE);
	delaySeconds(7);
}

void Search::setMode(Mode mode) {
	_mode = mode;

	if (_mode == OPTIONS)
		openNumbers(3);
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
