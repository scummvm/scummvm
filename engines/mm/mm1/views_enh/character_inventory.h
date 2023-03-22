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

#ifndef MM1_VIEWS_ENH_CHARACTER_INVENTORY_H
#define MM1_VIEWS_ENH_CHARACTER_INVENTORY_H

#include "mm/mm1/views_enh/items_view.h"
#include "mm/mm1/data/character.h"
#include "mm/mm1/game/equip_remove.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

class CharacterInventory : public ItemsView, Game::EquipRemove {
private:
	enum DisplayMode {
		ARMS_MODE, BACKPACK_MODE
	};
	DisplayMode _mode = ARMS_MODE;
	enum SelectedButton {
		BTN_NONE, BTN_EQUIP, BTN_REMOVE, BTN_DISCARD
	};
	SelectedButton _selectedButton = BTN_NONE;
	Character *_initialChar = nullptr;

	/**
	 * Populates the list of items
	 */
	void populateItems();

	/**
	 * Displays the title row
	 */
	void drawTitle();

	/**
	 * Selects a button mode
	 */
	void selectButton(SelectedButton btnMode);

	/**
	 * Handle action with selected button mode and selected item
	 */
	void performAction();

	/**
	 * Equip an item
	 */
	void equipItem();

	/**
	 * Unequip an item
	 */
	void removeItem();

	/**
	 * Discard an item
	 */
	void discardItem();

	/**
	 * Trade an item to another character
	 */
	void tradeItem(Character *from);

protected:
	/**
	 * Called when an item is selected
	 */
	void itemSelected() override;

	/**
	 * When the selected character is changed
	 */
	void selectedCharChanged() override;

public:
	CharacterInventory();
	virtual ~CharacterInventory() {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
