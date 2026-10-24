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
#include "mm/mm1/data/text.h"
#include "mm/mm1/views_enh/locations/blacksmith.h"
#include "mm/mm1/views_enh/confirm.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Locations {

BlacksmithItems::BlacksmithItems() : ItemsView("BlacksmithItems") {
	_btnSprites.load("buy.icn");
	addButton(0, "\x01""37Weap", Common::KEYCODE_w);
	addButton(2, "\x01""37Armor", Common::KEYCODE_a);
	addButton(6, "\x01""37Misc", Common::KEYCODE_m);
	addButton(10, "\x01""37Sell", Common::KEYCODE_s);
	addButton(12, "Exit", Common::KEYCODE_ESCAPE);
}

bool BlacksmithItems::msgFocus(const FocusMessage &msg) {
	ItemsView::msgFocus(msg);

	// When first opened, default to showing weapons to buy
	if (dynamic_cast<Blacksmith *>(msg._priorView) != nullptr) {
		_mode = WEAPONS_MODE;
		populateItems();
	}

	return true;
}

void BlacksmithItems::draw() {
	ItemsView::draw();
	drawTitle();
}

void BlacksmithItems::drawTitle() {
	const Character &c = *g_globals->_currCharacter;

	Common::String areaName = "Weapons";
	if (_mode == ARMOR_MODE)
		areaName = "Armor";
	if (_mode == MISC_MODE)
		areaName = "Misc";

	if (_mode == SELL_MODE) {
		// Show sell mode title
		Common::String title = Common::String::format(
			"Backpack for %s the %s",
			c._name,
			CLASS_NAMES[c._class - 1]
		);

		setReduced(false);
		writeLine(0, title, ALIGN_MIDDLE);

	} else {
		// Show title with "Available <Area>", "Gold", and "Cost"
		setReduced(false);
		Common::String title = Common::String::format("Available %s",
			areaName.c_str());
		writeString(0, 0, title);

		Common::String gold = Common::String::format("Gold - %d",
			c._gold);
		writeString(160, 0, gold);

		writeString(0, 0, "Cost", ALIGN_RIGHT);
	}
}

bool BlacksmithItems::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	switch (msg.keycode) {
	case Common::KEYCODE_w:
		_mode = WEAPONS_MODE;
		populateItems();
		redraw();
		break;
	case Common::KEYCODE_a:
		_mode = ARMOR_MODE;
		populateItems();
		redraw();
		break;
	case Common::KEYCODE_m:
		_mode = MISC_MODE;
		populateItems();
		redraw();
		break;
	case Common::KEYCODE_s:
		_mode = SELL_MODE;
		populateItems();
		redraw();
		break;

	default:
		return ItemsView::msgKeypress(msg);
	}

	return true;
}

bool BlacksmithItems::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	return ItemsView::msgAction(msg);
}

void BlacksmithItems::populateItems() {
	_costMode = (_mode == SELL_MODE) ? SHOW_VALUE : SHOW_COST;
	_items.clear();

	if (_mode != SELL_MODE) {
		// Populate the list of items that can be purchased
		// from the blacksmith in that category
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

void BlacksmithItems::itemSelected() {
	Common::String buySell, gold;
	g_globals->_items.getItem(_items[_selectedItem]);
	const Item &item = g_globals->_currItem;
	_buySellItem = _selectedItem;

	if (_mode != SELL_MODE) {
		buySell = "Buy";
		gold = Common::String::format(
			"for %d gold",
			item._cost);
	} else {
		buySell = "Sell";
		gold = Common::String::format(
			"for %d gold",
			item.getSellCost());
	}

	Common::String msg = Common::String::format(
		"%s \x02""15%s\x02""00 %s?",
		buySell.c_str(), item._name.c_str(), gold.c_str());
	Confirm::show(msg, []() {
		BlacksmithItems *view = static_cast<BlacksmithItems *>(
			g_events->focusedView());
		view->itemConfirmed();
	});
}

void BlacksmithItems::charSwitched(Character *priorChar) {
	ItemsView::charSwitched(priorChar);
	populateItems();
	redraw();
}

void BlacksmithItems::itemConfirmed() {
	Character &c = *g_globals->_currCharacter;
	Inventory &inv = c._backpack;

	if (_mode == SELL_MODE) {
		// Give the character the item value, and remove from inventory
		c._gold += g_globals->_currItem.getSellCost();
		inv.removeAt(_buySellItem);
		populateItems();

	} else {
		auto buyResult = c.buyItem(_items[_buySellItem]);

		switch (buyResult) {
		case Character::BUY_BACKPACK_FULL:
			backpackFull();
			break;
		case Character::BUY_NOT_ENOUGH_GOLD:
			notEnoughGold();
			break;
		default:
			displayMessage("Thank you!");
			break;
		}
	}
}

int BlacksmithItems::getLineColor() const {
	const Character &c = *g_globals->_currCharacter;
	const Item &item = g_globals->_currItem;

	if (_mode == SELL_MODE) {
		return 0;
	} else {
		if (c._class != NONE && c._class <= ROBBER) {
			if (!(BLACKSMITH_CLASS_USAGE[c._class - 1] & item._disablements))
				return 0;
		}

		return 1;
	}
}

} // namespace Locations
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
