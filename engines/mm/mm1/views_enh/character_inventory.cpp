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

#include "mm/mm1/views_enh/character_inventory.h"
#include "mm/mm1/views_enh/which_item.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

CharacterInventory::CharacterInventory() : ItemsView("CharacterInventory") {
	_btnSprites.load("items.icn");
	addButton(2, STRING["enhdialogs.items.buttons.arms"], Common::KEYCODE_a);
	addButton(6, STRING["enhdialogs.items.buttons.backpack"], Common::KEYCODE_b);
	addButton(8, STRING["enhdialogs.items.buttons.equip"], Common::KEYCODE_e);
	addButton(10, STRING["enhdialogs.items.buttons.remove"], Common::KEYCODE_r);
	addButton(12, STRING["enhdialogs.items.buttons.discard"], Common::KEYCODE_d);
	addButton(6, STRING["enhdialogs.items.buttons.trade"], Common::KEYCODE_t);
	addButton(14, STRING["enhdialogs.items.buttons.use"], Common::KEYCODE_u);
	addButton(16, STRING["enhdialogs.misc.exit"], Common::KEYCODE_ESCAPE);
}

bool CharacterInventory::msgFocus(const FocusMessage &msg) {
	ItemsView::msgFocus(msg);
	assert(g_globals->_currCharacter);

	if (dynamic_cast<WhichItem *>(msg._priorView) == nullptr)
		_mode = BACKPACK_MODE;
	populateItems();

	return true;
}

bool CharacterInventory::msgGame(const GameMessage &msg) {
	if (msg._name == "ITEM" && msg._value >= 0 &&
			msg._value <= (int)_items.size()) {
		_selectedItem = msg._value;
		performAction();
		return true;
	} else if (msg._name == "TRADE") {
		_tradeMode = msg._stringValue;
		_tradeAmount = msg._value;
		addView("WhichCharacter");
		return true;
	} else if (msg._name == "TRADE_DEST") {
		if (msg._value != -1)
			trade(_tradeMode, _tradeAmount, &g_globals->_party[msg._value]);
		return true;
	}

	return ItemsView::msgGame(msg);
}

void CharacterInventory::draw() {
	ItemsView::draw();
	drawTitle();
}

void CharacterInventory::drawTitle() {
	const Character &c = *g_globals->_currCharacter;
	const Common::String fmt = STRING[(_mode == ARMS_MODE) ?
		"enhdialogs.items.arms_for" :
		"enhdialogs.items.backpack_for"];

	const Common::String title = Common::String::format(fmt.c_str(),
		c._name,
		STRING[Common::String::format("stats.classes.%d", c._class)].c_str()
	);

	setReduced(false);
	writeLine(0, title, ALIGN_MIDDLE);
}

bool CharacterInventory::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	switch (msg.keycode) {
	case Common::KEYCODE_a:
		_mode = ARMS_MODE;
		populateItems();
		redraw();
		break;
	case Common::KEYCODE_b:
		_mode = BACKPACK_MODE;
		populateItems();
		redraw();
		break;
	case Common::KEYCODE_e:
		selectButton(BTN_EQUIP);
		break;
	case Common::KEYCODE_r:
		selectButton(BTN_REMOVE);
		break;
	case Common::KEYCODE_d:
		selectButton(BTN_DISCARD);
		break;
	case Common::KEYCODE_t:
		addView("Trade");
		break;
	default:
		return ItemsView::msgKeypress(msg);
	}

	return true;
}

bool CharacterInventory::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	return ItemsView::msgAction(msg);
}

void CharacterInventory::populateItems() {
	_items.clear();
	_selectedItem = -1;
	_selectedButton = BTN_NONE;
	_initialChar = g_globals->_currCharacter;

	const Character &c = *g_globals->_currCharacter;
	const Inventory &inv = (_mode == ARMS_MODE) ? c._equipped : c._backpack;

	for (uint i = 0; i < inv.size(); ++i)
		_items.push_back(inv[i]._id);
}

void CharacterInventory::itemSelected() {
	// No implementation
}

void CharacterInventory::selectedCharChanged() {
	// When in combat, the current character can't be changed
	if (g_events->isInCombat()) {
		if (g_globals->_currCharacter != _initialChar) {
			g_globals->_currCharacter = _initialChar;
			g_events->send("GameParty", GameMessage("CHAR_HIGHLIGHT", (int)true));
		}
	} else if (_selectedItem != -1) {
		// Trade to another character
		tradeItem(_initialChar);

	} else {
		populateItems();
		redraw();
	}
}

void CharacterInventory::selectButton(SelectedButton btnMode) {
	_selectedButton = btnMode;

	if (_selectedItem != -1) {
		performAction();
	} else {
		Common::String btn = STRING["enhdialogs.items.equip"];
		if (btnMode == BTN_REMOVE)
			btn = STRING["enhdialogs.items.remove"];
		else if (btnMode == BTN_DISCARD)
			btn = STRING["enhdialogs.items.discard"];

		send("WhichItem", GameMessage("DISPLAY",
			Common::String::format("%s %s", btn.c_str(),
				STRING["enhdialogs.items.which_item"].c_str())
		));
	}
}

void CharacterInventory::performAction() {
	switch (_selectedButton) {
	case BTN_EQUIP:
		equipItem();
		break;
	case BTN_REMOVE:
		removeItem();
		break;

	case BTN_DISCARD:
		discardItem();
		break;

	default:
		error("No button selected");
		break;
	}
}

void CharacterInventory::equipItem() {
	Common::String errMsg;
	Common::Point textPos;

	if (EquipRemove::equipItem(_selectedItem, textPos, errMsg)) {
		_mode = ARMS_MODE;
		populateItems();
		redraw();
	} else {
		displayMessage(errMsg);
	}
}

void CharacterInventory::removeItem() {
	Common::String errMsg;
	Common::Point textPos;

	if (EquipRemove::removeItem(_selectedItem, textPos, errMsg)) {
		_mode = BACKPACK_MODE;
		populateItems();
		redraw();
	} else {
		displayMessage(errMsg);
	}
}

void CharacterInventory::discardItem() {
	Character &c = *g_globals->_currCharacter;
	Inventory &inv = (_mode == ARMS_MODE) ? c._equipped : c._backpack;

	inv.removeAt(_selectedItem);
	populateItems();
	redraw();
}

void CharacterInventory::tradeItem(Character *from) {
	if (g_globals->_currCharacter == _initialChar)
		return;

	// Get source and dest inventories
	Character &cSrc = *_initialChar;
	Inventory &iSrc = (_mode == ARMS_MODE) ? cSrc._equipped : cSrc._backpack;
	Character &cDest = *g_globals->_currCharacter;
	Inventory &iDest = cDest._backpack;

	if (iDest.full()) {
		g_globals->_currCharacter = _initialChar;
		backpackFull();

	} else {
		Inventory::Entry item = iSrc[_selectedItem];
		iSrc.removeAt(_selectedItem);
		iDest.add(item._id, item._charges);

		_mode = BACKPACK_MODE;
		populateItems();
		redraw();
	}
}

void CharacterInventory::trade(const Common::String &mode, int amount, Character *destChar) {
	assert(isFocused());

	// TODO: implement
	if (mode == "GEMS") {

	} else if (mode == "GOLD") {

	} else if (mode == "FOOD") {

	}
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
