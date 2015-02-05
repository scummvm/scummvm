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
	
	Character *startingChar = c;
	ItemCategory category = mode == ITEMMODE_4 || mode == ITEMMODE_COMBAT ? 
		CATEGORY_MISC : CATEGORY_WEAPON;
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
			if ((mode != ITEMMODE_CHAR_INFO || category != CATEGORY_MISC) && mode != ITEMMODE_6
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
					category == 3 ? BTN_USE : BTN_EQUIP, 
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

		Common::StringArray lines;
		int arr[40];
		Common::fill(&arr[0], &arr[40], 0);
		int var2 = -1;
	
		if (mode == ITEMMODE_CHAR_INFO || category != 3) {
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

			switch (category) {
			case CATEGORY_WEAPON: 
			case CATEGORY_ARMOR:
			case CATEGORY_ACCESSORY: {
				XeenItem &i = (category == CATEGORY_WEAPON) ? c->_weapons[idx] :
					((category == CATEGORY_ARMOR) ? c->_armor[idx] : c->_accessories[idx]);

				if (i._id) {
					if (mode == ITEMMODE_CHAR_INFO || mode == ITEMMODE_8
							|| mode == ITEMMODE_6 || mode == ITEMMODE_4) {
						lines.push_back(Common::String::format(ITEMS_DIALOG_LINE1, 
							arr[idx], idx + 1,
							c->assembleItemName(idx, arr[idx], category)));
					} else {
						lines.push_back(Common::String::format(ITEMS_DIALOG_LINE2,
							arr[idx], idx + 1,
							c->assembleItemName(idx, arr[idx], category),
							calcItemCost(c, idx, mode,
								mode == ITEMMODE_TO_GOLD ? 1 : startingChar->_skills[MERCHANT],
								category)
						));
					}

					DrawStruct &ds = _itemsDrawList[idx];
					ds._sprites = &_equipSprites;
					if (c->_weapons.passRestrictions(i._id, true))
						ds._frame = i._frame;
					else
						ds._frame = 14;
				} else if (_itemsDrawList[idx]._sprites == nullptr) {
					lines.push_back(NO_ITEMS_AVAILABLE);
				}
				break;
			}

			case CATEGORY_MISC: {
				XeenItem &i = c->_misc[idx];
				_itemsDrawList[idx]._sprites = nullptr;

				if (i._material == 0) {
					// No item
					if (idx == 0) {
						lines.push_back(NO_ITEMS_AVAILABLE);
					}
				} else {
					ItemsMode tempMode = mode;
					int skill = startingChar->_skills[MERCHANT];

					if (mode == ITEMMODE_CHAR_INFO || mode == ITEMMODE_8
							|| mode == ITEMMODE_6 || mode == ITEMMODE_4) {
						tempMode = ITEMMODE_6;
					} else if (mode == ITEMMODE_TO_GOLD) {
						skill = 1;
					}

					lines.push_back(Common::String::format(ITEMS_DIALOG_LINE2,
						arr[idx], idx + 1,
						c->assembleItemName(idx, arr[idx], category),
						calcItemCost(c, idx, tempMode, skill, category)
					));
				}
				break;
			}

			default:
				break;
			}
		}
		while (lines.size() < INV_ITEMS_TOTAL)
			lines.push_back("");

		// Draw out overall text and the list of items
		switch (mode) {
		case ITEMMODE_CHAR_INFO:
		case ITEMMODE_8:
			screen._windows[30].writeString(Common::String::format(X_FOR_THE_Y,
				category == CATEGORY_MISC ? "\x3l" : "\x3c",
				CATEGORY_NAMES[category], c->_name.c_str(), CLASS_NAMES[c->_class],
				category == CATEGORY_MISC ? FMT_CHARGES : " ",
				lines[0].c_str(), lines[1].c_str(), lines[2].c_str(), lines[3].c_str(),
				lines[4].c_str(), lines[5].c_str(), lines[6].c_str(), lines[7].c_str(),
				lines[8].c_str()
			));

		case ITEMMODE_BLACKSMITH: {
			// Original uses var in this block that's never set?!
			const int v1 = 0;
			screen._windows[30].writeString(Common::String::format(AVAILABLE_GOLD_COST,
				CATEGORY_NAMES[category], 
				v1 ? "" : "s", party._gold,
				lines[0].c_str(), lines[1].c_str(), lines[2].c_str(), lines[3].c_str(),
				lines[4].c_str(), lines[5].c_str(), lines[6].c_str(), lines[7].c_str(),
				lines[8].c_str()
			));
			break;
		}

		case ITEMMODE_2:
		case ITEMMODE_4:
		case ITEMMODE_6:
		case ITEMMODE_9:
		case ITEMMODE_10:
		case ITEMMODE_TO_GOLD:
			screen._windows[30].writeString(Common::String::format(X_FOR_Y,
				CATEGORY_NAMES[category], startingChar->_name.c_str(),
				(mode == ITEMMODE_4 || mode == ITEMMODE_6) ? CHARGES : COST,
				lines[0].c_str(), lines[1].c_str(), lines[2].c_str(), lines[3].c_str(),
				lines[4].c_str(), lines[5].c_str(), lines[6].c_str(), lines[7].c_str(),
				lines[8].c_str()
			));
			break;

		case ITEMMODE_3:
		case ITEMMODE_5:
			screen._windows[30].writeString(Common::String::format(X_FOR_Y_GOLD,
				CATEGORY_NAMES[category], c->_name.c_str(), party._gold, CHARGES,
				lines[0].c_str(), lines[1].c_str(), lines[2].c_str(), lines[3].c_str(),
				lines[4].c_str(), lines[5].c_str(), lines[6].c_str(), lines[7].c_str(),
				lines[8].c_str()
				));
			break;

		default:
			break;
		}

		// Draw the glyphs for the items
		screen._windows[0].drawList(_itemsDrawList, INV_ITEMS_TOTAL);
		screen._windows[0].update();

		if (var2 != -1) {
			int actionIndex = -1;
			switch (mode) {
			case ITEMMODE_BLACKSMITH:
				actionIndex = 0;
				break;
			case ITEMMODE_2:
				actionIndex = 1;
				break;
			case ITEMMODE_9:
				actionIndex = 3;
				break;
			case ITEMMODE_10:
				actionIndex = 2;
				break;
			default:
				break;
			}

			if (actionIndex >= 0) {
				doItemOptions(*c, actionIndex, var2, category, mode);
			}
		}

		// TODO
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
					i._frame = 1;
				else if (i._id <= 29 || i._id > 33)
					i._frame = 13;
				else
					i._frame = 4;
				break;
			}

			case 1: {
				XeenItem &i = _itemsCharacter._armor[idx];
				if (i._id <= 7)
					i._frame = 3;
				else if (i._id == 9)
					i._frame = 5;
				else if (i._id == 10)
					i._frame = 9;
				else if (i._id <= 12)
					i._frame = 10;
				else
					i._frame = 6;
				break;
			}

			case 2: {
				XeenItem &i = _itemsCharacter._accessories[idx];
				if (i._id == 1)
					i._id = 8;
				else if (i._id == 2)
					i._frame = 12;
				else if (i._id <= 7)
					i._frame = 7;
				else
					i._frame = 11;
				break;
			}
			
			default:
				break;
			}
		}
	}
}

/**
 * Calculate the cost of an item
 */
int ItemsDialog::calcItemCost(Character *c, int itemIndex, ItemsMode mode,
		int skillLevel, ItemCategory category) {
	int amount1 = 0, amount2 = 0, amount3 = 0, amount4 = 0;
	int result = 0;
	int level = skillLevel & 0x7f;

	switch (mode) {
	case ITEMMODE_BLACKSMITH:
		level = 0;
		break;
	case ITEMMODE_2:
	case ITEMMODE_TO_GOLD:
		level = level == 0 ? 1 : 0;
		break;
	case ITEMMODE_10:
		level = 2;
		break;
	case ITEMMODE_9:
		level = 3;
		break;
	default:
		break;
	}
	
	switch (category) {
	case CATEGORY_WEAPON:
	case CATEGORY_ARMOR:
	case CATEGORY_ACCESSORY: {
		// 0=Weapons, 1=Armor, 2=Accessories
		XeenItem &i = (mode == 0) ? c->_weapons[itemIndex] :
			(mode == 1 ? c->_armor[itemIndex] : c->_accessories[itemIndex]);
		amount1 = (mode == 0) ? WEAPON_BASE_COSTS[i._id] :
			(mode == 1 ? ARMOR_BASE_COSTS[i._id] : ACCESSORY_BASE_COSTS[i._id]);
		
		if (i._material > 36 && i._material < 59) {
			switch (i._material) {
			case 37:
				amount1 /= 10;
				break;
			case 38:
				amount1 /= 4;
				break;
			case 39:
				amount1 /= 2;
				break;
			case 40:
				amount1 /= 4;
				break;
			default:
				amount1 *= METAL_BASE_MULTIPLIERS[i._material - 37];
				break;
			}
		}

		if (i._material < 37)
			amount2 = ELEMENTAL_DAMAGE[i._material] * 100;
		else if (i._material > 58)
			amount3 = METAL_BASE_MULTIPLIERS[i._material] * 100;
		
		switch (mode) {
		case ITEMMODE_BLACKSMITH:
		case ITEMMODE_2:
		case ITEMMODE_9:
		case ITEMMODE_10:
		case ITEMMODE_TO_GOLD:
			result = (amount1 + amount2 + amount3 + amount4) / ITEM_SKILL_DIVISORS[level];
			if (!result)
				result = 1;
			break;
		default:
			break;
		}
		break;
	}

	case 3: {
		// Misc
		XeenItem &i = c->_misc[itemIndex];
		amount1 = MISC_MATERIAL_COSTS[i._material];
		amount4 = MISC_BASE_COSTS[i._id];
		
		switch (mode) {
		case ITEMMODE_BLACKSMITH:
		case ITEMMODE_2:
		case ITEMMODE_9:
		case ITEMMODE_10:
		case ITEMMODE_TO_GOLD:
			result = (amount1 + amount2 + amount3 + amount4) / ITEM_SKILL_DIVISORS[level];
			if (!result)
				result = 1;
			break;
		default:
			break;
		}
		break;
	}

	default:
		break;
	}

	return (mode == ITEMMODE_CHAR_INFO) ? 0 : result;
}

bool ItemsDialog::doItemOptions(Character &c, int actionIndex, int itemIndex, ItemCategory category,
		ItemsMode mode) {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	Spells &spells = *_vm->_spells;

	XeenItem *itemCategories[4] = { &c._weapons[0], &c._armor[0], &c._accessories[0], &c._misc[0] };
	XeenItem *items = itemCategories[category];
	if (!items[0]._id)
		return false;

	Window &w = screen._windows[11];
	SpriteResource escSprites;
	if (itemIndex < 0 || itemIndex > 8) {
		saveButtons();

		escSprites.load("esc.icn");
		addButton(Common::Rect(235, 111, 259, 131), Common::KEYCODE_ESCAPE, &escSprites);
		addButton(Common::Rect(8, 20, 263, 28), Common::KEYCODE_1, &escSprites, false);
		addButton(Common::Rect(8, 29, 263, 37), Common::KEYCODE_2, &escSprites, false);
		addButton(Common::Rect(8, 38, 263, 46), Common::KEYCODE_3, &escSprites, false);
		addButton(Common::Rect(8, 47, 263, 55), Common::KEYCODE_4, &escSprites, false);
		addButton(Common::Rect(8, 56, 263, 64), Common::KEYCODE_5, &escSprites, false);
		addButton(Common::Rect(8, 65, 263, 73), Common::KEYCODE_6, &escSprites, false);
		addButton(Common::Rect(8, 74, 263, 82), Common::KEYCODE_7, &escSprites, false);
		addButton(Common::Rect(8, 83, 263, 91), Common::KEYCODE_8, &escSprites, false);
		addButton(Common::Rect(8, 92, 263, 100), Common::KEYCODE_9, &escSprites, false);

		w.open();
		w.writeString(Common::String::format(WHICH_ITEM, ITEM_ACTIONS[actionIndex]));
		_iconSprites.draw(screen, 0, Common::Point(235, 111));
		w.update();

		while (!_vm->shouldQuit()) {
			while (!_buttonValue) {
				events.pollEventsAndWait();
				checkEvents(_vm);
				if (_vm->shouldQuit())
					return false;
			}

			if (_buttonValue == Common::KEYCODE_ESCAPE) {
				itemIndex = -1;
				break;
			} else if (_buttonValue >= Common::KEYCODE_1 && _buttonValue <= Common::KEYCODE_9) {
				// Check whether there's an item at the selected index
				int selectedIndex = _buttonValue - Common::KEYCODE_1;
				if (!items[selectedIndex]._id)
					continue;

				itemIndex = selectedIndex;
				break;
			}
		}

		w.close();
		restoreButtons();
	}

	if (itemIndex != -1) {
		switch (mode) {
		case ITEMMODE_CHAR_INFO:
		case ITEMMODE_8:
			switch (actionIndex) {
			case 0:
				c._items[category].equipItem(itemIndex);
				break;
			case 1:
				c._items[category].removeItem(itemIndex);
				break;
			case 2:
				if (!party._mazeId) {
					ErrorScroll::show(_vm, WHATS_YOUR_HURRY);
				} else {
					XeenItem &i = c._misc[itemIndex];

					Condition condition = c.worstCondition();
					switch (condition) {
					case SLEEP:
					case PARALYZED:
					case UNCONSCIOUS:
					case DEAD:
					case STONED:
					case ERADICATED:
						ErrorScroll::show(_vm, Common::String::format(IN_NO_CONDITION, c._name.c_str()));
						break;
					default:
						if (combat._itemFlag) {
							ErrorScroll::show(_vm, USE_ITEM_IN_COMBAT);
						} else if (i._id && (i._bonusFlags & ITEMFLAG_BONUS_MASK)
								&& !(i._bonusFlags & (ITEMFLAG_BROKEN | ITEMFLAG_CURSED))) {
							int bonus = (i._bonusFlags & ITEMFLAG_BONUS_MASK) - 1;
							i._bonusFlags = bonus;
							_oldCharacter = &c;

							screen._windows[30].close();
							screen._windows[29].close();
							screen._windows[24].close();
							spells.doSpell(i._id);

							if (!bonus) {
								c._items[category].discardItem(itemIndex);
							}
						} else {
							ErrorScroll::show(_vm, Common::String::format(NO_SPECIAL_ABILITIES,
								c.assembleItemName(itemIndex, 15, category).c_str()
							));
						}
					}
				}
			case 3:
				// TODO: Remaining switches
			default:
				break;
			}
		}
	}

	intf._charsShooting = false;
	intf.moveMonsters();
	combat._whosTurn = -1;
	return true;
}

} // End of namespace Xeen
