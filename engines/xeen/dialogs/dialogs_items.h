/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef XEEN_DIALOGS_ITEMS_H
#define XEEN_DIALOGS_ITEMS_H

#include "xeen/dialogs/dialogs.h"
#include "xeen/party.h"
#include "xeen/window.h"

namespace Xeen {

enum ItemsMode {
	ITEMMODE_CHAR_INFO = 0, ITEMMODE_BUY = 1, ITEMMODE_SELL = 2, ITEMMODE_3 = 3,
	ITEMMODE_RECHARGE = 4, ITEMMODE_5 = 5, ITEMMODE_ENCHANT = 6, ITEMMODE_COMBAT = 7, ITEMMODE_8 = 8,
	ITEMMODE_REPAIR = 9, ITEMMODE_IDENTIFY = 10, ITEMMODE_TO_GOLD = 11,
	ITEMMODE_INVALID = -1
};

class ItemsDialog : public ButtonContainer {
private:
	SpriteResource _iconSprites;
	SpriteResource _equipSprites;
	Character _itemsCharacter;
	Character *_oldCharacter;
	DrawStruct _itemsDrawList[INV_ITEMS_TOTAL];

	ItemsDialog(XeenEngine *vm) : ButtonContainer(vm), _oldCharacter(nullptr) {}

	Character *execute(Character *c, ItemsMode mode);

	/**
	 * Load the buttons for the dialog
	 */
	void loadButtons(ItemsMode mode, Character *&c, ItemCategory category);

	/**
	 * Sets the equipment icon to use for each item for display
	 */
	void setEquipmentIcons();

	/**
	 * Calculate the cost of an item, or charges renaming for Misc items as appropriate
	 */
	int calcItemCost(Character *c, int itemIndex, ItemsMode mode, int skillLevel,
		ItemCategory category);

	int doItemOptions(Character &c, int actionIndex, int itemIndex,
		ItemCategory category, ItemsMode mode);

	void itemToGold(Character &c, int itemIndex, ItemCategory category, ItemsMode mode);
public:
	static Character *show(XeenEngine *vm, Character *c, ItemsMode mode);
};

class ItemSelectionDialog : public ButtonContainer {
private:
	SpriteResource _icons;
	int _actionIndex;
	InventoryItems &_items;

	ItemSelectionDialog(XeenEngine *vm, int actionIndex, InventoryItems &items) : ButtonContainer(vm),
			_actionIndex(actionIndex), _items(items) {
		loadButtons();
	}

	/**
	 * Executes the dialog
	 * @returns					Selected item index
	 */
	int execute();

	/**
	 * Loads buttons
	 */
	void loadButtons();
public:
	/**
	 * Shows the dialog
	 * @param actionIndex		Current action type
	 * @param items				Currently active items category
	 * @returns					Selected item index
	 */
	static int show(int actionIndex, InventoryItems &items);
};


} // End of namespace Xeen

#endif /* XEEN_DIALOGS_ITEMS_H */
