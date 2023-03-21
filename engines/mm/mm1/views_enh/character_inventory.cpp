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

	if (dynamic_cast<WhichItem *>(msg._priorView) == nullptr)
		_mode = ARMS_MODE;
	populateItems();

	return true;
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

	const Character &c = *g_globals->_currCharacter;
	const Inventory &inv = (_mode == ARMS_MODE) ? c._equipped : c._backpack;

	for (uint i = 0; i < inv.size(); ++i)
		_items.push_back(inv[i]._id);
}

void CharacterInventory::itemSelected() {
	// No implementation
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
