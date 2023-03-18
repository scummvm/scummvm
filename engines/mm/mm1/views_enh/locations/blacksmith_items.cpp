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

#include "mm/mm1/views_enh/locations/blacksmith_items.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Locations {

BlacksmithItems::BlacksmithItems() : ItemsView("BlacksmithItems") {
	_btnSprites.load("buy.icn");
	addButton(0, STRING["enhdialogs.blacksmith.buttons.weapons"], Common::KEYCODE_w);
	addButton(2, STRING["enhdialogs.blacksmith.buttons.armor"], Common::KEYCODE_a);
	addButton(6, STRING["enhdialogs.blacksmith.buttons.misc"], Common::KEYCODE_m);
	addButton(8, STRING["enhdialogs.blacksmith.buttons.buy"], Common::KEYCODE_b);
	addButton(10, STRING["enhdialogs.blacksmith.buttons.sell"], Common::KEYCODE_s);
	addButton(12, STRING["enhdialogs.blacksmith.buttons.exit"], Common::KEYCODE_ESCAPE);
}

bool BlacksmithItems::msgFocus(const FocusMessage &msg) {
	ItemsView::msgFocus(msg);

	_mode = WEAPONS_MODE;
	_buyMode = true;
	populateItems();

	return true;
}

void BlacksmithItems::draw() {
	ItemsView::draw();
	drawTitle();
}

void BlacksmithItems::drawTitle() {
	const Character &c = *g_globals->_currCharacter;

	Common::String areaName = STRING["enhdialogs.blacksmith.areas.weapons"];
	if (_mode == ARMOR_MODE)
		areaName = STRING["enhdialogs.blacksmith.areas.armor"];
	if (_mode == MISC_MODE)
		areaName = STRING["enhdialogs.blacksmith.areas.misc"];

	if (_buyMode) {
		// Show title with "Available <Area>", "Gold", and "Cost"
		setReduced(false);
		Common::String title = Common::String::format("%s %s",
			STRING["enhdialogs.blacksmith.available"].c_str(),
			areaName.c_str());
		writeString(0, 0, title);

		Common::String gold = Common::String::format("%s - %d",
			STRING["enhdialogs.blacksmith.gold"].c_str(),
			c._gold);
		writeString(160, 0, gold);

		writeString(0, 0, STRING["enhdialogs.blacksmith.cost"], ALIGN_RIGHT);

	} else {
		// Shows a title like "Weapons for Arturius the Paladin"
		Common::String title = Common::String::format(
			STRING["enhdialogs.blacksmith.backpack_for"].c_str(),
			c._name,
			STRING[Common::String::format("stats.classes.%d", c._class)].c_str()
		);

		setReduced(false);
		writeLine(0, title, ALIGN_MIDDLE);
	}
}

bool BlacksmithItems::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	return ItemsView::msgKeypress(msg);
}

bool BlacksmithItems::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	return ItemsView::msgAction(msg);
}

void BlacksmithItems::populateItems() {
	_startingChar = _buyMode ? '1' : 'A';
	_costMode = _buyMode ? SHOW_COST : SHOW_VALUE;
	_items.clear();

	if (_buyMode) {
		// Populate the list of items that can be purchased
		// from the Blacksmith in that category
		int townNum = g_maps->_currentMap->dataByte(Maps::MAP_ID);
		if (townNum < 1 || townNum >= 6)
			townNum = 1;

		const byte *ITEMS = WEAPONS[townNum - 1];
		if (_mode == ARMOR_MODE)
			ITEMS = ARMOR[townNum - 1];
		if (_mode == MISC_MODE)
			ITEMS = MISC[townNum - 1];

		for (int i = 0; i < 6; ++i)
			_items.push_back(ITEMS[i]);
	} else {
		// Sell mode, so list the player's inventory
		const Character &c = *g_globals->_currCharacter;

		for (uint i = 0; i < c._backpack.size(); ++i)
			_items.push_back(c._backpack[i]._id);
	}
}

} // namespace Locations
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
