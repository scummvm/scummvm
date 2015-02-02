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

#include "xeen/dialogs_items.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

Character *ItemsDialog::show(XeenEngine *vm, Character *c, ItemsMode mode) {
	ItemsDialog *dlg = new ItemsDialog(vm);
	Character *result = dlg->execute(c, mode);
	delete dlg;

	return result;
}

Character *ItemsDialog::execute(Character *c, ItemsMode mode) {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	
	Character *tempChar = c;
	int typeNum = mode == ITEMMODE_4 || mode == ITEMMODE_COMBAT ? 3 : 0;
	int varA = mode == ITEMMODE_COMBAT ? 1 : 0;
	if (varA != 0)
		mode = ITEMMODE_CHAR_INFO;
	int varE = mode == 1 ? 1 : 0;

	events.setCursor(0);
	loadButtons(mode, c);

	screen._windows[29].open();
	screen._windows[30].open();

	bool redrawFlag = true;
	while (!_vm->shouldQuit()) {
		if (redrawFlag) {
			if ((mode != ITEMMODE_CHAR_INFO || typeNum != 3) && mode != ITEMMODE_6
					&& mode != ITEMMODE_4 && mode != ITEMMODE_TO_GOLD) {
				_buttons[8]._bounds.moveTo(148, _buttons[8]._bounds.top);
				_buttons[9]._draw = false;
			} else if (mode == ITEMMODE_4) {
				_buttons[4]._value = Common::KEYCODE_r;
			} else if (mode == ITEMMODE_6) {
				_buttons[4]._value = Common::KEYCODE_e;
			} else if (mode == ITEMMODE_TO_GOLD) {
				_buttons[4]._value = Common::KEYCODE_g;
			} else {
				_buttons[8]._bounds.moveTo(0, _buttons[8]._bounds.top);
				_buttons[9]._draw = true;
				_buttons[9]._value = Common::KEYCODE_u;
			}

			// Write text for the dialog
			Common::String msg;
			if (mode != ITEMMODE_CHAR_INFO && mode != ITEMMODE_8 && mode != ITEMMODE_6
					&& mode != ITEMMODE_4 && mode != ITEMMODE_TO_GOLD) {
				msg = Common::String::format(ITEMS_DIALOG_TEXT1,
					BTN_SELL, BTN_IDENTIFY, BTN_FIX);
			} else if (mode != ITEMMODE_6  && mode != ITEMMODE_4 && mode != ITEMMODE_TO_GOLD) {
				msg = Common::String::format(ITEMS_DIALOG_TEXT1,
					typeNum == 3 ? BTN_USE : BTN_EQUIP, 
					BTN_REMOVE, BTN_DISCARD, BTN_QUEST);
			} else if (mode == ITEMMODE_6) {
				msg = Common::String::format(ITEMS_DIALOG_TEXT2, BTN_ENCHANT);
			} else if (mode == ITEMMODE_4) {
				msg = Common::String::format(ITEMS_DIALOG_TEXT2, BTN_RECHARGE);
			} else {
				msg = Common::String::format(ITEMS_DIALOG_TEXT2, BTN_GOLD);
			}

			screen._windows[29].writeString(msg);
			drawButtons(&screen);
		}

		int arr[40];
		Common::fill(&arr[0], &arr[40], false);
		int var2 = -1;
	
		if (mode == ITEMMODE_CHAR_INFO || typeNum != 3) {
			_iconSprites.draw(screen, 8, Common::Point(148, 109));
		}
		if (mode != ITEMMODE_6 && mode != ITEMMODE_4 && mode != ITEMMODE_TO_GOLD) {
			_iconSprites.draw(screen, 10, Common::Point(182, 109));
			_iconSprites.draw(screen, 12, Common::Point(216, 109));
			_iconSprites.draw(screen, 14, Common::Point(250, 109));
		}

		switch (mode) {
		case ITEMMODE_CHAR_INFO:
			_iconSprites.draw(screen, 9, Common::Point(148, 109));
			break;
		case ITEMMODE_BLACKSMITH:
			_iconSprites.draw(screen, 11, Common::Point(182, 109));
			break;
		case ITEMMODE_9:
			_iconSprites.draw(screen, 15, Common::Point(250, 109));
			break;
		case ITEMMODE_10:
			_iconSprites.draw(screen, 13, Common::Point(216, 109));
			break;
		default:
			break;
		}

		for (int idx = 0; idx < 9; ++idx) {
			_itemsDrawList[idx]._y = 10 + idx * 9;

			switch (typeNum) {
			case 0:
				if (c->_weapons[idx]._id) {
					if (mode == ITEMMODE_CHAR_INFO || mode == ITEMMODE_8
							|| mode == ITEMMODE_6 || mode == ITEMMODE_4) {
						// TODO
					}
				} else if (_itemsDrawList[idx]._sprites == nullptr) {
					// TODO
				}
				break;
				// TODO
			default:
				break;
			}
		}
	}

	return c;
}

/**
 * Load the buttons for the dialog
 */
void ItemsDialog::loadButtons(ItemsMode mode, Character *&c) {
	_iconSprites.load(Common::String::format("%s.icn",
		(mode == ITEMMODE_CHAR_INFO) ? "items" : "buy"));
	_equipSprites.load("equip.icn");

	if (mode == ITEMMODE_6 || mode == ITEMMODE_4 || mode == ITEMMODE_TO_GOLD) {
		// Enchant button list
		addButton(Common::Rect(12, 109, 36, 129), Common::KEYCODE_w, &_iconSprites);
		addButton(Common::Rect(46, 109, 70, 129), Common::KEYCODE_a, &_iconSprites);
		addButton(Common::Rect(80, 109, 104, 129), Common::KEYCODE_c, &_iconSprites);
		addButton(Common::Rect(114, 109, 138, 129), Common::KEYCODE_n, &_iconSprites);
		addButton(Common::Rect(148, 109, 172, 129), Common::KEYCODE_e, &_iconSprites);
		addButton(Common::Rect(284, 109, 308, 129), Common::KEYCODE_ESCAPE, &_iconSprites);
		addButton(Common::Rect(148, 109, 172, 129), Common::KEYCODE_u, &_iconSprites);
		addButton(Common::Rect(8, 20, 263, 28), Common::KEYCODE_1, &_iconSprites, false);
		addButton(Common::Rect(8, 29, 263, 37), Common::KEYCODE_2, &_iconSprites, false);
		addButton(Common::Rect(8, 38, 263, 46), Common::KEYCODE_3, &_iconSprites, false);
		addButton(Common::Rect(8, 47, 263, 55), Common::KEYCODE_4, &_iconSprites, false);
		addButton(Common::Rect(8, 56, 263, 64), Common::KEYCODE_5, &_iconSprites, false);
		addButton(Common::Rect(8, 65, 263, 73), Common::KEYCODE_6, &_iconSprites, false);
		addButton(Common::Rect(8, 74, 263, 82), Common::KEYCODE_7, &_iconSprites, false);
		addButton(Common::Rect(8, 83, 263, 91), Common::KEYCODE_8, &_iconSprites, false);
		addButton(Common::Rect(8, 92, 263, 100), Common::KEYCODE_9, &_iconSprites, false);
	} else {
		addButton(Common::Rect(12, 109, 36, 129), Common::KEYCODE_w, &_iconSprites);
		addButton(Common::Rect(46, 109, 70, 129), Common::KEYCODE_a, &_iconSprites);
		addButton(Common::Rect(80, 109, 104, 129), Common::KEYCODE_c, &_iconSprites);
		addButton(Common::Rect(114, 109, 138, 129), Common::KEYCODE_n, &_iconSprites);
		addButton(Common::Rect(148, 109, 172, 129), Common::KEYCODE_e, &_iconSprites);
		addButton(Common::Rect(182, 109, 206, 129), Common::KEYCODE_r, &_iconSprites);
		addButton(Common::Rect(216, 109, 240, 129), Common::KEYCODE_d, &_iconSprites);
		addButton(Common::Rect(250, 109, 274, 129), Common::KEYCODE_q, &_iconSprites);
		addButton(Common::Rect(284, 109, 308, 129), Common::KEYCODE_ESCAPE, &_iconSprites);
		addButton(Common::Rect(8, 20, 263, 28), Common::KEYCODE_1, &_iconSprites, false);
		addButton(Common::Rect(8, 29, 263, 37), Common::KEYCODE_2, &_iconSprites, false);
		addButton(Common::Rect(8, 38, 263, 46), Common::KEYCODE_3, &_iconSprites, false);
		addButton(Common::Rect(8, 47, 263, 55), Common::KEYCODE_4, &_iconSprites, false);
		addButton(Common::Rect(8, 56, 263, 64), Common::KEYCODE_5, &_iconSprites, false);
		addButton(Common::Rect(8, 65, 263, 73), Common::KEYCODE_6, &_iconSprites, false);
		addButton(Common::Rect(8, 74, 263, 82), Common::KEYCODE_7, &_iconSprites, false);
		addButton(Common::Rect(8, 83, 263, 91), Common::KEYCODE_8, &_iconSprites, false);
		addButton(Common::Rect(8, 92, 263, 100), Common::KEYCODE_9, &_iconSprites, false);
	}

	if (mode == ITEMMODE_BLACKSMITH) {
		_oldCharacter = c;
		c = &_itemsCharacter;
		blackData2CharData();

		_buttons[4]._value = Common::KEYCODE_b;
		_buttons[5]._value = Common::KEYCODE_s;
		_buttons[6]._value = Common::KEYCODE_i;
		_buttons[7]._value = Common::KEYCODE_f;

		setEquipmentIcons();
	} else {
		_buttons[4]._value = Common::KEYCODE_e;
		_buttons[5]._value = Common::KEYCODE_r;
		_buttons[6]._value = Common::KEYCODE_d;
		_buttons[7]._value = Common::KEYCODE_q;
	}
}

/**
 * Loads the temporary _itemsCharacter character with the item set
 * the given blacksmith has available, so the user can "view" the
 * set as if it were a standard character's inventory
 */
void ItemsDialog::blackData2CharData() {
	Party &party = *_vm->_party;
	bool isDarkCc = _vm->_files->_isDarkCc;
	int slotIndex = 0;
	while (party._mazeId != (int)BLACKSMITH_MAP_IDS[isDarkCc][slotIndex] && slotIndex < 4)
		++slotIndex;
	if (slotIndex == 4)
		slotIndex = 0;

	for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
		_itemsCharacter._weapons[idx] = party._blacksmithWeapons[isDarkCc][idx];
		_itemsCharacter._armor[idx] = party._blacksmithArmor[isDarkCc][idx];
		_itemsCharacter._accessories[idx] = party._blacksmithAccessories[isDarkCc][idx];
		_itemsCharacter._misc[idx] = party._blacksmithMisc[isDarkCc][idx];
	}
}

/**
 * Sets the equipment icon to use for each item for display
 */
void ItemsDialog::setEquipmentIcons() {
	for (int typeIndex = 0; typeIndex < 4; ++typeIndex) {
		for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
			switch (typeIndex) {
			case 0: {
				XeenItem &i = _itemsCharacter._weapons[idx];
				if (i._id <= 17)
					i._equipped = 1;
				else if (i._id <= 29 || i._id > 33)
					i._equipped = 13;
				else
					i._equipped = 4;
				break;
			}

			case 1: {
				XeenItem &i = _itemsCharacter._armor[idx];
				if (i._id <= 7)
					i._equipped = 3;
				else if (i._id == 9)
					i._equipped = 5;
				else if (i._id == 10)
					i._equipped = 9;
				else if (i._id <= 12)
					i._equipped = 10;
				else
					i._equipped = 6;
				break;
			}

			case 2: {
				XeenItem &i = _itemsCharacter._accessories[idx];
				if (i._id == 1)
					i._id = 8;
				else if (i._id == 2)
					i._equipped = 12;
				else if (i._id <= 7)
					i._equipped = 7;
				else
					i._equipped = 11;
				break;
			}
			
			default:
				break;
			}
		}
	}
}

} // End of namespace Xeen
