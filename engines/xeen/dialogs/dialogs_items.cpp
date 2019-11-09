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

#include "xeen/dialogs/dialogs_items.h"
#include "xeen/dialogs/dialogs_query.h"
#include "xeen/dialogs/dialogs_quests.h"
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
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Windows &windows = *_vm->_windows;

	ItemsMode priorMode = ITEMMODE_INVALID;
	Character *startingChar = c;
	ItemCategory category = mode == ITEMMODE_RECHARGE || mode == ITEMMODE_COMBAT ?
		CATEGORY_MISC : CATEGORY_WEAPON;
	int varA = mode == ITEMMODE_COMBAT ? 1 : 0;
	if (varA != 0)
		mode = ITEMMODE_CHAR_INFO;
	bool updateStock = mode == ITEMMODE_BUY;
	int itemIndex = -1;
	Common::StringArray lines;
	uint arr[40];
	int actionIndex = -1;

	if (mode == ITEMMODE_BUY) {
		_oldCharacter = c;
		c = &_itemsCharacter;
		party._blacksmithWares.blackData2CharData(_itemsCharacter);
		setEquipmentIcons();
	} else if (mode == ITEMMODE_ENCHANT) {
		_oldCharacter = c;
	}

	events.setCursor(0);
	windows[29].open();
	windows[30].open();

	enum { REDRAW_NONE, REDRAW_TEXT, REDRAW_FULL } redrawFlag = REDRAW_FULL;
	for (;;) {
		if (redrawFlag == REDRAW_FULL) {
			// Write text for the dialog
			Common::String msg;
			if (mode != ITEMMODE_CHAR_INFO && mode != ITEMMODE_8 && mode != ITEMMODE_ENCHANT
					&& mode != ITEMMODE_RECHARGE && mode != ITEMMODE_TO_GOLD) {
				msg = Common::String::format(Res.ITEMS_DIALOG_TEXT1,
					Res.BTN_BUY, Res.BTN_SELL, Res.BTN_IDENTIFY, Res.BTN_FIX);
			} else if (mode != ITEMMODE_ENCHANT  && mode != ITEMMODE_RECHARGE && mode != ITEMMODE_TO_GOLD) {
				msg = Common::String::format(Res.ITEMS_DIALOG_TEXT1,
					category == 3 ? Res.BTN_USE : Res.BTN_EQUIP,
					Res.BTN_REMOVE, Res.BTN_DISCARD, Res.BTN_QUEST);
			} else if (mode == ITEMMODE_ENCHANT) {
				msg = Common::String::format(Res.ITEMS_DIALOG_TEXT2, Res.BTN_ENCHANT);
			} else if (mode == ITEMMODE_RECHARGE) {
				msg = Common::String::format(Res.ITEMS_DIALOG_TEXT2, Res.BTN_RECHARGE);
			} else {
				msg = Common::String::format(Res.ITEMS_DIALOG_TEXT2, Res.BTN_GOLD);
			}

			windows[29].writeString(msg);

			Common::fill(&arr[0], &arr[40], 0);
			itemIndex = -1;
			priorMode = ITEMMODE_INVALID;
		}

		if (mode != priorMode) {
			// Set up the buttons for the dialog
			loadButtons(mode, c, category);
			priorMode = mode;
			drawButtons(&windows[0]);
		}

		if (redrawFlag == REDRAW_TEXT || redrawFlag == REDRAW_FULL) {
			lines.clear();

			for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
				DrawStruct &ds = _itemsDrawList[idx];
				XeenItem &i = c->_items[category][idx];

				ds._sprites = nullptr;
				ds._x = 8;
				ds._y = 18 + idx * 9;

				switch (category) {
				case CATEGORY_WEAPON:
				case CATEGORY_ARMOR:
				case CATEGORY_ACCESSORY:
					if (i._id) {
						if ((mode == ITEMMODE_CHAR_INFO && !g_vm->_extOptions._showItemCosts)
								|| mode == ITEMMODE_8 || mode == ITEMMODE_ENCHANT || mode == ITEMMODE_RECHARGE) {
							lines.push_back(Common::String::format(Res.ITEMS_DIALOG_LINE1,
								arr[idx], idx + 1,
								c->_items[category].getFullDescription(idx, arr[idx]).c_str()));
						} else {
							lines.push_back(Common::String::format(Res.ITEMS_DIALOG_LINE2,
								arr[idx], idx + 1,
								c->_items[category].getFullDescription(idx, arr[idx]).c_str(),
								calcItemCost(c, idx,
									(mode == ITEMMODE_CHAR_INFO) ? ITEMMODE_BUY : mode,
									mode == ITEMMODE_TO_GOLD ? 1 : startingChar->_skills[MERCHANT],
									category)
							));
						}

						ds._sprites = &_equipSprites;
						if (c->_items[category].passRestrictions(i._id, true))
							ds._frame = i._frame;
						else
							ds._frame = 14;
					} else if (ds._sprites == nullptr && idx == 0) {
						lines.push_back(Res.NO_ITEMS_AVAILABLE);
					}
					break;

				case CATEGORY_MISC:
					if (i._material == 0) {
						// No item
						if (idx == 0) {
							lines.push_back(Res.NO_ITEMS_AVAILABLE);
						}
					} else {
						ItemsMode tempMode = mode;
						int skill = startingChar->_skills[MERCHANT];

						if (mode == ITEMMODE_CHAR_INFO || mode == ITEMMODE_8
								|| mode == ITEMMODE_ENCHANT || mode == ITEMMODE_RECHARGE) {
							tempMode = ITEMMODE_ENCHANT;
						} else if (mode == ITEMMODE_TO_GOLD) {
							skill = 1;
						}

						lines.push_back(Common::String::format(Res.ITEMS_DIALOG_LINE2,
							arr[idx], idx + 1,
							c->_items[category].getFullDescription(idx, arr[idx]).c_str(),
							calcItemCost(c, idx, tempMode, skill, category)
						));
					}
					break;

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
				windows[30].writeString(Common::String::format(Res.X_FOR_THE_Y,
					category == CATEGORY_MISC ? "\x3l" : "\x3""c",
					Res.CATEGORY_NAMES[category], c->_name.c_str(), Res.CLASS_NAMES[c->_class],
					category == CATEGORY_MISC ? Res.FMT_CHARGES : " ",
					lines[0].c_str(), lines[1].c_str(), lines[2].c_str(), lines[3].c_str(),
					lines[4].c_str(), lines[5].c_str(), lines[6].c_str(), lines[7].c_str(),
					lines[8].c_str()
				));
				break;

			case ITEMMODE_BUY:
				windows[30].writeString(Common::String::format(Res.AVAILABLE_GOLD_COST,
					Res.CATEGORY_NAMES[category], party._gold,
					lines[0].c_str(), lines[1].c_str(), lines[2].c_str(), lines[3].c_str(),
					lines[4].c_str(), lines[5].c_str(), lines[6].c_str(), lines[7].c_str(),
					lines[8].c_str()
				));
				break;

			case ITEMMODE_SELL:
			case ITEMMODE_RECHARGE:
			case ITEMMODE_ENCHANT:
			case ITEMMODE_REPAIR:
			case ITEMMODE_IDENTIFY:
			case ITEMMODE_TO_GOLD:
				windows[30].writeString(Common::String::format(Res.X_FOR_Y,
					Res.CATEGORY_NAMES[category], startingChar->_name.c_str(),
					(mode == ITEMMODE_RECHARGE || mode == ITEMMODE_ENCHANT) ? Res.CHARGES : Res.COST,
					lines[0].c_str(), lines[1].c_str(), lines[2].c_str(), lines[3].c_str(),
					lines[4].c_str(), lines[5].c_str(), lines[6].c_str(), lines[7].c_str(),
					lines[8].c_str()
				));
				break;

			case ITEMMODE_3:
			case ITEMMODE_5:
				windows[30].writeString(Common::String::format(Res.X_FOR_Y_GOLD,
					Res.CATEGORY_NAMES[category], c->_name.c_str(), party._gold, Res.CHARGES,
					lines[0].c_str(), lines[1].c_str(), lines[2].c_str(), lines[3].c_str(),
					lines[4].c_str(), lines[5].c_str(), lines[6].c_str(), lines[7].c_str(),
					lines[8].c_str()
					));
				break;

			default:
				break;
			}

			// Draw the glyphs for the items
			windows[0].drawList(_itemsDrawList, INV_ITEMS_TOTAL);
			windows[0].update();
		}

		redrawFlag = REDRAW_NONE;

		if (itemIndex != -1) {
			switch (mode) {
			case ITEMMODE_BUY:
				actionIndex = 0;
				break;
			case ITEMMODE_SELL:
				actionIndex = 1;
				break;
			case ITEMMODE_REPAIR:
				actionIndex = 3;
				break;
			case ITEMMODE_IDENTIFY:
				actionIndex = 2;
				break;
			default:
				break;
			}
		}

		// If it's time to do an item action, take care of it
		if (actionIndex >= 0) {
			int result = doItemOptions(*c, actionIndex, itemIndex, category, mode);
			if (result == 1) {
				// Finish dialog with no selected character
				c = nullptr;
				break;
			} else if (result == 2) {
				// Close dialogs and finish dialog with original starting character
				windows[30].close();
				windows[29].close();
				c = startingChar;
				break;
			}

			// Otherwise, result and continue showing dialog
			actionIndex = -1;
			redrawFlag = REDRAW_FULL;
			continue;
		}

		// Wait for a selection
		_buttonValue = 0;
		while (!_vm->shouldExit() && !_buttonValue) {
			events.pollEventsAndWait();
			checkEvents(_vm);
		}
		if (_vm->shouldExit())
			return nullptr;

		// Handle escaping out of dialog
		if (_buttonValue == Common::KEYCODE_ESCAPE) {
			if (mode == ITEMMODE_8)
				continue;
			c = startingChar;
			break;
		}

		// Handle other selections
		switch (_buttonValue) {
		case Common::KEYCODE_F1:
		case Common::KEYCODE_F2:
		case Common::KEYCODE_F3:
		case Common::KEYCODE_F4:
		case Common::KEYCODE_F5:
		case Common::KEYCODE_F6:
			if (!varA && mode != ITEMMODE_3 && mode != ITEMMODE_ENCHANT
					&& mode != ITEMMODE_RECHARGE && mode != ITEMMODE_TO_GOLD
					&& party._mazeId != 0) {
				_buttonValue -= Common::KEYCODE_F1;

				if (_buttonValue < (int)(_vm->_mode == MODE_COMBAT ?
						combat._combatParty.size() : party._activeParty.size())) {
					// Character number is valid
					redrawFlag = REDRAW_FULL;
					Character *newChar = _vm->_mode == MODE_COMBAT ?
						combat._combatParty[_buttonValue] : &party._activeParty[_buttonValue];

					if (mode == ITEMMODE_BUY) {
						_oldCharacter = newChar;
						startingChar = newChar;
						c = &_itemsCharacter;
					} else if (mode == ITEMMODE_SELL || mode == ITEMMODE_REPAIR || mode == ITEMMODE_IDENTIFY) {
						_oldCharacter = newChar;
						startingChar = newChar;
						c = newChar;
					} else if (itemIndex != -1) {
						// Switching item to another character
						InventoryItems &destItems = newChar->_items[category];
						InventoryItems &srcItems = c->_items[category];
						XeenItem &srcItem = srcItems[itemIndex];

						if (srcItem._state._cursed)
							ErrorScroll::show(_vm, Res.CANNOT_REMOVE_CURSED_ITEM);
						else if (destItems.isFull())
							ErrorScroll::show(_vm, Common::String::format(
								Res.CATEGORY_BACKPACK_IS_FULL[category], c->_name.c_str()));
						else {
							XeenItem &destItem = destItems[INV_ITEMS_TOTAL - 1];
							destItem = srcItem;
							srcItem.clear();
							destItem._frame = 0;

							srcItems.sort();
							destItems.sort();
							continue;
						}
					} else {
						c = newChar;
						startingChar = newChar;
					}

					intf.highlightChar(_buttonValue);
					continue;
				}
			}
			break;

		case Common::KEYCODE_1:
		case Common::KEYCODE_2:
		case Common::KEYCODE_3:
		case Common::KEYCODE_4:
		case Common::KEYCODE_5:
		case Common::KEYCODE_6:
		case Common::KEYCODE_7:
		case Common::KEYCODE_8:
		case Common::KEYCODE_9:
			// Select an item
			if (mode == ITEMMODE_3)
				break;

			_buttonValue -= Common::KEYCODE_1;
			if (_buttonValue != itemIndex) {
				// Check whether the new selection has an associated item
				if (!c->_items[category][_buttonValue].empty()) {
					itemIndex = _buttonValue;
					Common::fill(&arr[0], &arr[40], 0);
					arr[itemIndex] = 15;
				}
			} else {
				Common::fill(&arr[0], &arr[40], 0);
				itemIndex = -1;
			}

			redrawFlag = REDRAW_TEXT;
			break;

		case Common::KEYCODE_a:
			// Armor category
			category = CATEGORY_ARMOR;
			redrawFlag = REDRAW_FULL;
			break;

		case Common::KEYCODE_b:
			// Buy
			if (mode != ITEMMODE_CHAR_INFO && mode != ITEMMODE_RECHARGE &&
					mode != ITEMMODE_ENCHANT && mode != ITEMMODE_TO_GOLD) {
				mode = ITEMMODE_BUY;
				c = &_itemsCharacter;
				redrawFlag = REDRAW_FULL;
			}
			break;

		case Common::KEYCODE_c:
			// Accessories category
			category = CATEGORY_ACCESSORY;
			redrawFlag = REDRAW_FULL;
			break;

		case Common::KEYCODE_d:
			if (mode == ITEMMODE_CHAR_INFO)
				actionIndex = 3;
			break;

		case Common::KEYCODE_e:
			if (mode == ITEMMODE_CHAR_INFO || mode == ITEMMODE_ENCHANT ||
					mode == ITEMMODE_TO_GOLD) {
				if (category != CATEGORY_MISC) {
					actionIndex = mode == ITEMMODE_ENCHANT ? 4 : 0;
				}
			}
			break;

		case Common::KEYCODE_f:
			if (mode != ITEMMODE_CHAR_INFO && mode != ITEMMODE_RECHARGE &&
					mode != ITEMMODE_ENCHANT && mode != ITEMMODE_TO_GOLD) {
				mode = ITEMMODE_REPAIR;
				c = startingChar;
				redrawFlag = REDRAW_TEXT;
			}
			break;

		case Common::KEYCODE_g:
			if (mode == ITEMMODE_TO_GOLD)
				actionIndex = 6;
			break;

		case Common::KEYCODE_i:
			if (mode != ITEMMODE_CHAR_INFO && mode != ITEMMODE_RECHARGE &&
					mode != ITEMMODE_ENCHANT && mode != ITEMMODE_TO_GOLD) {
				mode = ITEMMODE_IDENTIFY;
				c = startingChar;
				redrawFlag = REDRAW_TEXT;
			}
			break;

		case Common::KEYCODE_m:
			// Misc
			category = CATEGORY_MISC;
			redrawFlag = REDRAW_FULL;
			break;

		case Common::KEYCODE_q:
			// Quests
			if (mode == ITEMMODE_CHAR_INFO) {
				Quests::show(_vm);
				redrawFlag = REDRAW_FULL;
			}
			break;

		case Common::KEYCODE_r:
			if (mode == ITEMMODE_CHAR_INFO)
				actionIndex = 1;
			else if (mode == ITEMMODE_RECHARGE)
				actionIndex = 5;
			break;

		case Common::KEYCODE_s:
			if (mode != ITEMMODE_CHAR_INFO && mode != ITEMMODE_RECHARGE &&
					mode != ITEMMODE_ENCHANT && mode != ITEMMODE_TO_GOLD) {
				mode = ITEMMODE_SELL;
				c = startingChar;
				redrawFlag = REDRAW_TEXT;
			}
			break;

		case Common::KEYCODE_u:
			if (mode == ITEMMODE_CHAR_INFO && category == CATEGORY_MISC)
				actionIndex = 2;
			break;

		case Common::KEYCODE_w:
			// Weapons category
			category = CATEGORY_WEAPON;
			redrawFlag = REDRAW_FULL;
			break;

		default:
			break;
		}
	}

	windows[30].close();
	windows[29].close();

	intf.drawParty(true);
	if (updateStock)
		party._blacksmithWares.charData2BlackData(_itemsCharacter);

	return c;
}

void ItemsDialog::loadButtons(ItemsMode mode, Character *&c, ItemCategory category) {
	if (_iconSprites.empty())
		_iconSprites.load(Common::String::format("%s.icn", (mode == ITEMMODE_CHAR_INFO) ? "items" : "buy"));
	if (_equipSprites.empty())
		_equipSprites.load("equip.icn");

	clearButtons();
	if (mode == ITEMMODE_ENCHANT || mode == ITEMMODE_RECHARGE || mode == ITEMMODE_TO_GOLD) {
		// Enchant button list
		addButton(Common::Rect(12, 109, 36, 129), Common::KEYCODE_w, &_iconSprites);
		addButton(Common::Rect(46, 109, 70, 129), Common::KEYCODE_a, &_iconSprites);
		addButton(Common::Rect(80, 109, 104, 129), Common::KEYCODE_c, &_iconSprites);
		addButton(Common::Rect(114, 109, 138, 129), Common::KEYCODE_m, &_iconSprites);
		addButton(Common::Rect(148, 109, 172, 129), Common::KEYCODE_e, &_iconSprites);
		addButton(Common::Rect(284, 109, 308, 129), Common::KEYCODE_ESCAPE, &_iconSprites);
		addButton(Common::Rect(148, 109, 172, 129), Common::KEYCODE_u, &_iconSprites);
		addButton(Common::Rect(8, 20, 263, 28), Common::KEYCODE_1);
		addButton(Common::Rect(8, 29, 263, 37), Common::KEYCODE_2);
		addButton(Common::Rect(8, 38, 263, 46), Common::KEYCODE_3);
		addButton(Common::Rect(8, 47, 263, 55), Common::KEYCODE_4);
		addButton(Common::Rect(8, 56, 263, 64), Common::KEYCODE_5);
		addButton(Common::Rect(8, 65, 263, 73), Common::KEYCODE_6);
		addButton(Common::Rect(8, 74, 263, 82), Common::KEYCODE_7);
		addButton(Common::Rect(8, 83, 263, 91), Common::KEYCODE_8);
		addButton(Common::Rect(8, 92, 263, 100), Common::KEYCODE_9);
	} else {
		bool flag = mode == ITEMMODE_BUY || mode == ITEMMODE_SELL || mode == ITEMMODE_IDENTIFY
			|| mode == ITEMMODE_REPAIR;
		addButton(Common::Rect(12, 109, 36, 129), Common::KEYCODE_w, &_iconSprites);
		addButton(Common::Rect(46, 109, 70, 129), Common::KEYCODE_a, &_iconSprites);
		addButton(Common::Rect(80, 109, 104, 129), Common::KEYCODE_c, &_iconSprites);
		addButton(Common::Rect(114, 109, 138, 129), Common::KEYCODE_m, &_iconSprites);
		addButton(Common::Rect(148, 109, 172, 129), flag ? Common::KEYCODE_b : Common::KEYCODE_e, &_iconSprites);
		addButton(Common::Rect(182, 109, 206, 129), flag ? Common::KEYCODE_s : Common::KEYCODE_r, &_iconSprites);
		addButton(Common::Rect(216, 109, 240, 129), flag ? Common::KEYCODE_i : Common::KEYCODE_d, &_iconSprites);
		addButton(Common::Rect(250, 109, 274, 129), flag ? Common::KEYCODE_f : Common::KEYCODE_q, &_iconSprites);
		addButton(Common::Rect(284, 109, 308, 129), Common::KEYCODE_ESCAPE, &_iconSprites);
		addButton(Common::Rect(8, 20, 263, 28), Common::KEYCODE_1);
		addButton(Common::Rect(8, 29, 263, 37), Common::KEYCODE_2);
		addButton(Common::Rect(8, 38, 263, 46), Common::KEYCODE_3);
		addButton(Common::Rect(8, 47, 263, 55), Common::KEYCODE_4);
		addButton(Common::Rect(8, 56, 263, 64), Common::KEYCODE_5);
		addButton(Common::Rect(8, 65, 263, 73), Common::KEYCODE_6);
		addButton(Common::Rect(8, 74, 263, 82), Common::KEYCODE_7);
		addButton(Common::Rect(8, 83, 263, 91), Common::KEYCODE_8);
		addButton(Common::Rect(8, 92, 263, 100), Common::KEYCODE_9);
		addPartyButtons(_vm);
	}

	if (mode == ITEMMODE_CHAR_INFO && category == CATEGORY_MISC) {
		_buttons[4].setFrame(18);
		_buttons[4]._value = Common::KEYCODE_u;
	}
	if (mode != ITEMMODE_ENCHANT && mode != ITEMMODE_RECHARGE && mode != ITEMMODE_TO_GOLD) {
		_buttons[5].setFrame(10);
		_buttons[6].setFrame(12);
		_buttons[7].setFrame(14);
	}

	// Set button as depressed depending on which mode the dialog is currently in
	switch (mode) {
	case ITEMMODE_BUY:
		_buttons[4].setFrame(9);
		break;
	case ITEMMODE_SELL:
		_buttons[5].setFrame(11);
		break;
	case ITEMMODE_IDENTIFY:
		_buttons[6].setFrame(13);
		break;
	case ITEMMODE_REPAIR:
		_buttons[7].setFrame(15);
		break;
	default:
		break;
	}

	if ((mode != ITEMMODE_CHAR_INFO || category != CATEGORY_MISC) && mode != ITEMMODE_ENCHANT
			&& mode != ITEMMODE_RECHARGE && mode != ITEMMODE_TO_GOLD) {
		_buttons[4]._bounds.moveTo(148, _buttons[4]._bounds.top);
		_buttons[9]._draw = false;
	} else if (mode == ITEMMODE_RECHARGE) {
		_buttons[4]._value = Common::KEYCODE_r;
	} else if (mode == ITEMMODE_ENCHANT) {
		_buttons[4]._value = Common::KEYCODE_e;
	} else if (mode == ITEMMODE_TO_GOLD) {
		_buttons[4]._value = Common::KEYCODE_g;
	}
}

void ItemsDialog::setEquipmentIcons() {
	for (int typeIndex = 0; typeIndex < 4; ++typeIndex) {
		for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
			switch (typeIndex) {
			case CATEGORY_WEAPON: {
				XeenItem &i = _itemsCharacter._weapons[idx];
				if (i._id <= 17)
					i._frame = 1;
				else if (i._id <= 29 || i._id > 33)
					i._frame = 13;
				else
					i._frame = 4;
				break;
			}

			case CATEGORY_ARMOR: {
				XeenItem &i = _itemsCharacter._armor[idx];
				if (i._id <= 7)
					i._frame = 3;
				else if (i._id == 8)
					i._frame = 2;
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

			case CATEGORY_ACCESSORY: {
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

int ItemsDialog::calcItemCost(Character *c, int itemIndex, ItemsMode mode,
		int skillLevel, ItemCategory category) {
	int amount1 = 0, amount2 = 0, amount3 = 0, amount4 = 0;
	int result = 0;
	int level = skillLevel & 0x7f;
	XeenItem &i = c->_items[category][itemIndex];

	const int *BASE_COSTS[4] = {
		Res.WEAPON_BASE_COSTS, Res.ARMOR_BASE_COSTS, Res.ACCESSORY_BASE_COSTS, Res.MISC_BASE_COSTS
	};

	switch (mode) {
	case ITEMMODE_BUY:
		level = 0;
		break;
	case ITEMMODE_SELL:
	case ITEMMODE_TO_GOLD:
		level = level == 0 ? 1 : 0;
		break;
	case ITEMMODE_IDENTIFY:
		level = 2;
		break;
	case ITEMMODE_REPAIR:
		level = 3;
		break;
	default:
		break;
	}

	switch (category) {
	case CATEGORY_WEAPON:
	case CATEGORY_ARMOR:
	case CATEGORY_ACCESSORY:
		amount1 = (BASE_COSTS[category])[i._id];

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
				amount1 *= Res.METAL_BASE_MULTIPLIERS[i._material - 37];
				break;
			}
		}

		if (i._material < 37)
			amount2 = Res.ELEMENTAL_DAMAGE[i._material] * 100;
		else if (i._material > 58)
			amount3 = Res.ELEMENTAL_DAMAGE[i._material - 59 + 7] * 100;

		switch (mode) {
		case ITEMMODE_BUY:
		case ITEMMODE_SELL:
		case ITEMMODE_REPAIR:
		case ITEMMODE_IDENTIFY:
		case ITEMMODE_TO_GOLD:
			result = (amount1 + amount2 + amount3 + amount4) / Res.ITEM_SKILL_DIVISORS[level];
			if (!result)
				result = 1;
			break;
		default:
			break;
		}
		break;

	case CATEGORY_MISC:
		// Misc
		amount1 = Res.MISC_MATERIAL_COSTS[i._material];
		amount4 = Res.MISC_BASE_COSTS[i._id];

		switch (mode) {
		case ITEMMODE_BUY:
		case ITEMMODE_SELL:
		case ITEMMODE_REPAIR:
		case ITEMMODE_IDENTIFY:
		case ITEMMODE_TO_GOLD:
			result = (amount1 + amount2 + amount3 + amount4) / Res.ITEM_SKILL_DIVISORS[level];
			if (!result)
				result = 1;
			break;

		case ITEMMODE_3:
		case ITEMMODE_RECHARGE:
		case ITEMMODE_5:
		case ITEMMODE_ENCHANT:
			// Show number of charges
			result = i._state._counter;
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return (mode == ITEMMODE_CHAR_INFO) ? 0 : result;
}

int ItemsDialog::doItemOptions(Character &c, int actionIndex, int itemIndex, ItemCategory category,
		ItemsMode mode) {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;
	Spells &spells = *_vm->_spells;
	Windows &windows = *_vm->_windows;
	int ccNum = _vm->_files->_ccNum;

	InventoryItems &items = c._items[category];
	if (items[0].empty())
		// Inventory is empty
		return category == CATEGORY_MISC ? 0 : 2;

	if (itemIndex < 0 || itemIndex > 8)
		itemIndex = ItemSelectionDialog::show(actionIndex, items);

	if (itemIndex != -1) {
		XeenItem &item = items[itemIndex];

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
					ErrorScroll::show(_vm, Res.WHATS_YOUR_HURRY);
				} else {
					XeenItem &i = c._misc[itemIndex];

					Condition condition = c.worstCondition();
					switch (condition) {
					case ASLEEP:
					case PARALYZED:
					case UNCONSCIOUS:
					case DEAD:
					case STONED:
					case ERADICATED:
						ErrorScroll::show(_vm, Common::String::format(Res.IN_NO_CONDITION, c._name.c_str()));
						break;
					default:
						if (combat._itemFlag) {
							ErrorScroll::show(_vm, Res.USE_ITEM_IN_COMBAT);
						} else if (i._id && !i.isBad() && i._state._counter > 0) {
							--i._state._counter;
							_oldCharacter = &c;

							windows[30].close();
							windows[29].close();
							windows[24].close();
							spells.castItemSpell(i._id);

							if (!i._state._counter) {
								// Ran out of charges, so make item disappear
								c._items[category][itemIndex].clear();
								c._items[category].sort();
							}

							intf._charsShooting = false;
							combat.moveMonsters();
							combat._whosTurn = -1;
							return 1;
						} else {
							ErrorScroll::show(_vm, Common::String::format(Res.NO_SPECIAL_ABILITIES,
								c._items[category].getFullDescription(itemIndex).c_str()
							));
						}
					}
				}
				break;
			case 3:
				if (c._items[category].discardItem(itemIndex) && mode == ITEMMODE_8)
					return 2;
				break;
			default:
				break;
			}
			break;

		case ITEMMODE_BUY: {
			InventoryItems &invItems = _oldCharacter->_items[category];
			if (invItems.isFull()) {
				// Character's inventory for that category is already full
				ErrorScroll::show(_vm, Common::String::format(Res.BACKPACK_IS_FULL,
					_oldCharacter->_name.c_str()));
			} else {
				int cost = calcItemCost(&c, itemIndex, mode, 0, category);
				Common::String desc = c._items[category].getFullDescription(itemIndex);
				if (Confirm::show(_vm, Common::String::format(Res.BUY_X_FOR_Y_GOLD,
						desc.c_str(), cost))) {
					if (party.subtract(CONS_GOLD, cost, WHERE_PARTY, WT_FREEZE_WAIT)) {
						if (ccNum) {
							sound.stopSound();
							sound.playSound("choice2.voc");
						}

						// Add entry to the end of the list
						XeenItem &srcItem = c._items[category][itemIndex];
						XeenItem &destItem = _oldCharacter->_items[category][INV_ITEMS_TOTAL - 1];
						destItem = srcItem;
						destItem._frame = 0;

						srcItem.clear();
						c._items[category].sort();
						_oldCharacter->_items[category].sort();
					}
				}
			}
			break;
		}

		case ITEMMODE_SELL: {
			bool noNeed;
			switch (category) {
			case CATEGORY_WEAPON:
				noNeed = (item._state._cursed) || item._id >= XEEN_SLAYER_SWORD;
				break;
			default:
				noNeed = item._state._cursed;
				break;
			}

			if (noNeed) {
				ErrorScroll::show(_vm, Common::String::format(Res.NO_NEED_OF_THIS,
					c._items[category].getFullDescription(itemIndex).c_str()));
			} else {
				int cost = calcItemCost(&c, itemIndex, mode, c._skills[MERCHANT], category);
				Common::String desc = c._items[category].getFullDescription(itemIndex);
				Common::String msg = Common::String::format(Res.SELL_X_FOR_Y_GOLD,
					desc.c_str(), cost);

				if (Confirm::show(_vm, msg)) {
					// Remove the sold item and add gold to the party's total
					item.clear();
					c._items[category].sort();

					party._gold += cost;
				}
			}
			break;
		}

		case ITEMMODE_RECHARGE:
			if (category != CATEGORY_MISC || item.empty() || item._material > 9 || item._id == 53) {
				sound.playFX(21);
				ErrorScroll::show(_vm, Common::String::format(Res.NOT_RECHARGABLE, Res.SPELL_FAILED));
			} else {
				item._state._counter = MIN(63, _vm->getRandomNumber(1, 6) + item._state._counter);
				sound.playFX(20);
			}
			return 2;

		case ITEMMODE_ENCHANT: {
			int amount = _vm->getRandomNumber(1, _oldCharacter->getCurrentLevel() / 5 + 1);
			amount = MIN(amount, 5);
			_oldCharacter->_items[category].enchantItem(itemIndex, amount);
			return 2;
		}

		case ITEMMODE_REPAIR:
			if (!item._state._broken) {
				ErrorScroll::show(_vm, Res.ITEM_NOT_BROKEN);
			} else {
				int cost = calcItemCost(&c, itemIndex, mode, actionIndex, category);
				Common::String msg = Common::String::format(Res.FIX_IDENTIFY_GOLD,
					Res.FIX_IDENTIFY[0],
					c._items[category].getFullDescription(itemIndex).c_str(),
					cost);

				if (Confirm::show(_vm, msg) && party.subtract(CONS_GOLD, cost, WHERE_PARTY)) {
					item._state._broken = false;
				}
			}
			break;

		case ITEMMODE_IDENTIFY: {
			int cost = calcItemCost(&c, itemIndex, mode, actionIndex, category);
			Common::String msg = Common::String::format(Res.FIX_IDENTIFY_GOLD,
				Res.FIX_IDENTIFY[1],
				c._items[category].getFullDescription(itemIndex).c_str(),
				cost);

			if (Confirm::show(_vm, msg) && party.subtract(CONS_GOLD, cost, WHERE_PARTY)) {
				Common::String details = c._items[category].getIdentifiedDetails(itemIndex);
				Common::String desc = c._items[category].getFullDescription(itemIndex);
				Common::String str = Common::String::format(Res.IDENTIFY_ITEM_MSG,
					desc.c_str(), details.c_str());

				Window &win = windows[14];
				win.open();
				win.writeString(str);
				win.update();

				saveButtons();
				clearButtons();

				while (!_vm->shouldExit() && !events.isKeyMousePressed())
					events.pollEventsAndWait();
				events.clearEvents();

				restoreButtons();
				win.close();
			}
			break;
		}

		case ITEMMODE_TO_GOLD:
			// Convert item in inventory to gold
			itemToGold(c, itemIndex, category, mode);
			return 2;

		default:
			break;
		}
	}

	return 0;
}

void ItemsDialog::itemToGold(Character &c, int itemIndex, ItemCategory category,
		ItemsMode mode) {
	XeenItem &item = c._items[category][itemIndex];
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	if (category == CATEGORY_WEAPON && item._id >= XEEN_SLAYER_SWORD) {
		sound.playFX(21);
		ErrorScroll::show(_vm, Common::String::format("\v012\t000\x03""c%s",
			Res.SPELL_FAILED));
	} else if (!item.empty()) {
		// There is a valid item present
		// Calculate cost of item and add it to the party's total
		int cost = calcItemCost(&c, itemIndex, mode, 1, category);
		party._gold += cost;

		// Remove the item from the inventory
		item.clear();
		c._items[category].sort();
	}
}

/*------------------------------------------------------------------------*/

int ItemSelectionDialog::show(int actionIndex, InventoryItems &items) {
	ItemSelectionDialog *dlg = new ItemSelectionDialog(g_vm, actionIndex, items);
	int result = dlg->execute();
	delete dlg;

	return result;
}

void ItemSelectionDialog::loadButtons() {
	_icons.load("esc.icn");
	addButton(Common::Rect(235, 111, 259, 131), Common::KEYCODE_ESCAPE, &_icons);
	addButton(Common::Rect(8, 20, 263, 28), Common::KEYCODE_1);
	addButton(Common::Rect(8, 29, 263, 37), Common::KEYCODE_2);
	addButton(Common::Rect(8, 38, 263, 46), Common::KEYCODE_3);
	addButton(Common::Rect(8, 47, 263, 55), Common::KEYCODE_4);
	addButton(Common::Rect(8, 56, 263, 64), Common::KEYCODE_5);
	addButton(Common::Rect(8, 65, 263, 73), Common::KEYCODE_6);
	addButton(Common::Rect(8, 74, 263, 82), Common::KEYCODE_7);
	addButton(Common::Rect(8, 83, 263, 91), Common::KEYCODE_8);
	addButton(Common::Rect(8, 92, 263, 100), Common::KEYCODE_9);
}

int ItemSelectionDialog::execute() {
	EventsManager &events = *g_vm->_events;
	Windows &windows = *g_vm->_windows;
	Window &w = windows[13];

	w.open();
	w.writeString(Common::String::format(Res.WHICH_ITEM, Res.ITEM_ACTIONS[_actionIndex]));
	_icons.draw(0, 0, Common::Point(235, 111));
	w.update();

	int itemIndex = -1;
	while (!_vm->shouldExit()) {
		_buttonValue = 0;
		while (!_buttonValue) {
			events.pollEventsAndWait();
			checkEvents(_vm);
			if (_vm->shouldExit())
				return false;
		}

		if (_buttonValue == Common::KEYCODE_ESCAPE) {
			itemIndex = -1;
			break;
		}
		else if (_buttonValue >= Common::KEYCODE_1 && _buttonValue <= Common::KEYCODE_9) {
			// Check whether there's an item at the selected index
			int selectedIndex = _buttonValue - Common::KEYCODE_1;
			if (!_items[selectedIndex]._id)
				continue;

			itemIndex = selectedIndex;
			break;
		}
	}

	w.close();
	return itemIndex;
}

} // End of namespace Xeen
