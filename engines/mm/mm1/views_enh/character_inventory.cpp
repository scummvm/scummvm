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
#include "mm/mm1/views_enh/combat.h"
#include "mm/mm1/views_enh/game_messages.h"
#include "mm/mm1/views_enh/trade.h"
#include "mm/mm1/views_enh/which_item.h"
#include "mm/mm1/data/locations.h"
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

	bool inCombat = g_events->isInCombat();
	for (int i = 2; i < 7; ++i)
		setButtonEnabled(i, !inCombat);

	if (dynamic_cast<WhichItem *>(msg._priorView) == nullptr &&
		dynamic_cast<Trade *>(msg._priorView) == nullptr &&
		dynamic_cast<GameMessages *>(msg._priorView) == nullptr) {
		_mode = BACKPACK_MODE;
		_selectedButton = BTN_NONE;
	}
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
	} else if (msg._name == "USE") {
		// Combat use item mode
		addView();
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
		return true;
	case Common::KEYCODE_b:
		_mode = BACKPACK_MODE;
		populateItems();
		redraw();
		return true;
	default:
		break;
	}

	if (!g_events->isInCombat()) {
		switch (msg.keycode) {
		case Common::KEYCODE_e:
			selectButton(BTN_EQUIP);
			return true;
		case Common::KEYCODE_r:
			selectButton(BTN_REMOVE);
			return true;
		case Common::KEYCODE_d:
			selectButton(BTN_DISCARD);
			return true;
		case Common::KEYCODE_t:
			addView("Trade");
			return true;
		case Common::KEYCODE_u:
			selectButton(BTN_USE);
			return true;
		default:
			break;
		}
	}

	return ItemsView::msgKeypress(msg);
}

bool CharacterInventory::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	return ItemsView::msgAction(msg);
}

void CharacterInventory::populateItems() {
	_items.clear();
	_selectedItem = -1;

	const Character &c = *g_globals->_currCharacter;
	const Inventory &inv = (_mode == ARMS_MODE) ? c._equipped : c._backpack;

	for (uint i = 0; i < inv.size(); ++i)
		_items.push_back(inv[i]._id);
}

bool CharacterInventory::canSwitchChar() {
	// When in combat, the current character can't be changed
	return !g_events->isInCombat();
}

bool CharacterInventory::canSwitchToChar(Character *dst) {
	if (_selectedItem != -1) {
		tradeItem(dst);
		return false;
	}

	return true;
}

void CharacterInventory::charSwitched(Character *priorChar) {
	PartyView::charSwitched(priorChar);
	populateItems();
	redraw();
}

void CharacterInventory::itemSelected() {
	if (g_events->isInCombat() && dynamic_cast<Combat *>(g_events->priorView()) != nullptr) {
		useItem();
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
		else if (btnMode == BTN_USE)
			btn = STRING["enhdialogs.items.use"];

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

	case BTN_USE:
		useItem();
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

void CharacterInventory::useItem() {
	Character &c = *g_globals->_currCharacter;
	Inventory &inv = (_mode == ARMS_MODE) ? c._equipped : c._backpack;
	Inventory::Entry *invEntry = &inv[_selectedItem];
	Common::String msg;

	if (g_events->isInCombat())
		msg = Game::UseItem::combatUseItem(inv, *invEntry, _mode == BACKPACK_MODE);
	else
		msg = Game::UseItem::nonCombatUseItem(inv, *invEntry, _mode == BACKPACK_MODE);

	displayMessage(msg);
}

void CharacterInventory::discardItem() {
	Character &c = *g_globals->_currCharacter;
	Inventory &inv = (_mode == ARMS_MODE) ? c._equipped : c._backpack;

	inv.removeAt(_selectedItem);
	populateItems();
	redraw();
}

void CharacterInventory::tradeItem(Character *dst) {
	if (dst == g_globals->_currCharacter)
		return;

	// Get source and dest inventories
	Character &cSrc = *g_globals->_currCharacter;
	Inventory &iSrc = (_mode == ARMS_MODE) ? cSrc._equipped : cSrc._backpack;
	Character &cDest = *dst;
	Inventory &iDest = cDest._backpack;

	if (iDest.full()) {
		backpackFull();

	} else {
		Inventory::Entry item = iSrc[_selectedItem];
		iSrc.removeAt(_selectedItem);
		iDest.add(item._id, item._charges);

		populateItems();
		redraw();
	}
}

void CharacterInventory::trade(const Common::String &mode, int amount, Character *destChar) {
	assert(isFocused());
	Character &src = *g_globals->_currCharacter;

	if (mode == "GEMS") {
		src._gems -= amount;
		destChar->_gems = MIN(destChar->_gems + amount, 0xffff);

	} else if (mode == "GOLD") {
		src._gold -= amount;
		destChar->_gold += amount;

	} else if (mode == "FOOD") {
		src._food -= amount;
		destChar->_food = MIN(destChar->_food + amount, MAX_FOOD);
	}

	redraw();
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
