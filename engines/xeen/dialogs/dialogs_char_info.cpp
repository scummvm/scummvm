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

#include "xeen/dialogs/dialogs_awards.h"
#include "xeen/dialogs/dialogs_char_info.h"
#include "xeen/dialogs/dialogs_exchange.h"
#include "xeen/dialogs/dialogs_items.h"
#include "xeen/dialogs/dialogs_quick_ref.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

void CharacterInfo::show(XeenEngine *vm, int charIndex) {
	CharacterInfo *dlg = new CharacterInfo(vm);
	dlg->execute(charIndex);
	delete dlg;
}

void CharacterInfo::execute(int charIndex) {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Windows &windows = *_vm->_windows;

	bool redrawFlag = true;
	Mode oldMode = _vm->_mode;
	_vm->_mode = MODE_CHARACTER_INFO;
	loadDrawStructs();
	addButtons();

	Character *c = (oldMode != MODE_COMBAT) ? &party._activeParty[charIndex] : combat._combatParty[charIndex];
	intf.highlightChar(charIndex);
	Window &w = windows[24];
	w.open();

	do {
		if (redrawFlag) {
			Common::String charDetails = loadCharacterDetails(*c);
			w.writeString(Common::String::format(Res.CHARACTER_TEMPLATE, charDetails.c_str()));
			w.drawList(_drawList, 24);
			w.update();
			redrawFlag = false;
		}

		// Wait for keypress, showing a blinking cursor
		events.updateGameCounter();
		bool cursorFlag = false;
		_buttonValue = 0;
		while (!_vm->shouldExit() && !_buttonValue) {
			events.pollEventsAndWait();
			if (events.timeElapsed() > 4) {
				cursorFlag = !cursorFlag;
				events.updateGameCounter();
			}

			showCursor(cursorFlag);
			w.update();
			checkEvents(_vm);
		}
		events.clearEvents();

		switch (_buttonValue) {
		case Common::KEYCODE_F1:
		case Common::KEYCODE_F2:
		case Common::KEYCODE_F3:
		case Common::KEYCODE_F4:
		case Common::KEYCODE_F5:
		case Common::KEYCODE_F6:
			_buttonValue -= Common::KEYCODE_F1;
			if (_buttonValue < (int)(oldMode == MODE_COMBAT ? combat._combatParty.size() : party._activeParty.size())) {
				charIndex = _buttonValue;
				c = (oldMode != MODE_COMBAT) ? &party._activeParty[charIndex] : combat._combatParty[charIndex];

				intf.highlightChar(_buttonValue);
				redrawFlag = true;
			} else {
				_vm->_mode = MODE_CHARACTER_INFO;
			}
			break;

		case Common::KEYCODE_UP:
		case Common::KEYCODE_KP8:
			if (_cursorCell > 0) {
				showCursor(false);
				--_cursorCell;
				showCursor(true);
			}
			w.update();
			break;

		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_KP2:
			if (_cursorCell < 20) {
				showCursor(false);
				++_cursorCell;
				showCursor(true);
			}
			w.update();
			break;

		case Common::KEYCODE_LEFT:
		case Common::KEYCODE_KP4:
			if (_cursorCell >= 5) {
				showCursor(false);
				_cursorCell -= 5;
				showCursor(true);
			}
			w.update();
			break;

		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_KP6:
			if (_cursorCell <= 15) {
				showCursor(false);
				_cursorCell += 5;
				showCursor(true);
			}
			w.update();
			break;

		case 1001:
		case 1002:
		case 1003:
		case 1004:
		case 1005:
		case 1006:
		case 1007:
		case 1008:
		case 1009:
		case 1010:
		case 1011:
		case 1012:
		case 1013:
		case 1014:
		case 1015:
		case 1016:
		case 1017:
		case 1018:
		case 1019:
		case 1020:
			showCursor(false);
			_cursorCell = _buttonValue - 1001;
			showCursor(true);
			w.update();
			// fall through

		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER: {
			bool result = expandStat(_cursorCell, *c);
			_vm->_mode = MODE_COMBAT;
			if (result)
				redrawFlag = true;
			break;
		}

		case Common::KEYCODE_e:
			if (oldMode == MODE_COMBAT) {
				ErrorScroll::show(_vm, Res.EXCHANGING_IN_COMBAT, WT_FREEZE_WAIT);
			} else {
				_vm->_mode = oldMode;
				ExchangeDialog::show(_vm, c, charIndex);
				_vm->_mode = MODE_CHARACTER_INFO;
				redrawFlag = true;
			}
			break;

		case Common::KEYCODE_i:
			_vm->_mode = oldMode;
			_vm->_combat->_itemFlag = _vm->_mode == MODE_COMBAT;
			c = ItemsDialog::show(_vm, c, ITEMMODE_CHAR_INFO);

			if (!c) {
				party._stepped = true;
				goto exit;
			}

			_vm->_mode = MODE_CHARACTER_INFO;
			redrawFlag = true;
			break;

		case Common::KEYCODE_q:
			QuickReferenceDialog::show(_vm);
			redrawFlag = true;
			break;

		case Common::KEYCODE_ESCAPE:
			goto exit;

		default:
			break;
		}
	} while (!_vm->shouldExit());
exit:
	w.close();
	intf.unhighlightChar();
	_vm->_mode = oldMode;
	_vm->_combat->_itemFlag = false;
}

void CharacterInfo::loadDrawStructs() {
	_drawList[0] = DrawStruct(0, 2, 16);
	_drawList[1] = DrawStruct(2, 2, 39);
	_drawList[2] = DrawStruct(4, 2, 62);
	_drawList[3] = DrawStruct(6, 2, 85);
	_drawList[4] = DrawStruct(8, 2, 108);
	_drawList[5] = DrawStruct(10, 53, 16);
	_drawList[6] = DrawStruct(12, 53, 39);
	_drawList[7] = DrawStruct(14, 53, 62);
	_drawList[8] = DrawStruct(16, 53, 85);
	_drawList[9] = DrawStruct(18, 53, 108);
	_drawList[10] = DrawStruct(20, 104, 16);
	_drawList[11] = DrawStruct(22, 104, 39);
	_drawList[12] = DrawStruct(24, 104, 62);
	_drawList[13] = DrawStruct(26, 104, 85);
	_drawList[14] = DrawStruct(28, 104, 108);
	_drawList[15] = DrawStruct(30, 169, 16);
	_drawList[16] = DrawStruct(32, 169, 39);
	_drawList[17] = DrawStruct(34, 169, 62);
	_drawList[18] = DrawStruct(36, 169, 85);
	_drawList[19] = DrawStruct(38, 169, 108);
	_drawList[20] = DrawStruct(40, 277, 3);
	_drawList[21] = DrawStruct(42, 277, 35);
	_drawList[22] = DrawStruct(44, 277, 67);
	_drawList[23] = DrawStruct(46, 277, 99);

	_iconSprites.load("view.icn");
	for (int idx = 0; idx < 24; ++idx)
		_drawList[idx]._sprites = &_iconSprites;
}

void CharacterInfo::addButtons() {
	addButton(Common::Rect(10, 24, 34, 44), 1001, &_iconSprites);
	addButton(Common::Rect(10, 47, 34, 67), 1002, &_iconSprites);
	addButton(Common::Rect(10, 70, 34, 90), 1003, &_iconSprites);
	addButton(Common::Rect(10, 93, 34, 113), 1004, &_iconSprites);
	addButton(Common::Rect(10, 116, 34, 136), 1005, &_iconSprites);
	addButton(Common::Rect(61, 24, 85, 44), 1006, &_iconSprites);
	addButton(Common::Rect(61, 47, 85, 67), 1007, &_iconSprites);
	addButton(Common::Rect(61, 70, 85, 90), 1008, &_iconSprites);
	addButton(Common::Rect(61, 93, 85, 113), 1009, &_iconSprites);
	addButton(Common::Rect(61, 116, 85, 136), 1010, &_iconSprites);
	addButton(Common::Rect(112, 24, 136, 44), 1011, &_iconSprites);
	addButton(Common::Rect(112, 47, 136, 67), 1012, &_iconSprites);
	addButton(Common::Rect(112, 70, 136, 90), 1013, &_iconSprites);
	addButton(Common::Rect(112, 93, 136, 113), 1014, &_iconSprites);
	addButton(Common::Rect(112, 116, 136, 136), 1015, &_iconSprites);
	addButton(Common::Rect(177, 24, 201, 44), 1016, &_iconSprites);
	addButton(Common::Rect(177, 47, 201, 67), 1017, &_iconSprites);
	addButton(Common::Rect(177, 70, 201, 90), 1018, &_iconSprites);
	addButton(Common::Rect(177, 93, 201, 113), 1019, &_iconSprites);
	addButton(Common::Rect(177, 116, 201, 136), 1020, &_iconSprites);
	addButton(Common::Rect(285, 11, 309, 31), Common::KEYCODE_i, &_iconSprites);
	addButton(Common::Rect(285, 43, 309, 63), Common::KEYCODE_q, &_iconSprites);
	addButton(Common::Rect(285, 75, 309, 95), Common::KEYCODE_e, &_iconSprites);
	addButton(Common::Rect(285, 107, 309, 127), Common::KEYCODE_ESCAPE, &_iconSprites);
	addPartyButtons(_vm);
}

Common::String CharacterInfo::loadCharacterDetails(const Character &c) {
	Condition condition = c.worstCondition();
	Party &party = *_vm->_party;
	int foodVal = party._food / party._activeParty.size() / 3;

	int totalResist =
		c._fireResistence._permanent + c.itemScan(11) + c._fireResistence._temporary +
		c._coldResistence._permanent + c.itemScan(13) + c._coldResistence._temporary +
		c._electricityResistence._permanent + c.itemScan(12) + c._electricityResistence._temporary +
		c._poisonResistence._permanent + c.itemScan(14) + c._poisonResistence._temporary +
		c._energyResistence._permanent + c.itemScan(15) + c._energyResistence._temporary +
		c._magicResistence._permanent + c.itemScan(16) + c._magicResistence._temporary;

	return Common::String::format(Res.CHARACTER_DETAILS,
		Res.PARTY_GOLD, c._name.c_str(), Res.SEX_NAMES[c._sex],
		Res.RACE_NAMES[c._race], Res.CLASS_NAMES[c._class],
		c.statColor(c.getStat(MIGHT), c.getStat(MIGHT, true)), c.getStat(MIGHT),
		c.statColor(c.getStat(ACCURACY), c.getStat(ACCURACY, true)), c.getStat(ACCURACY),
		c.statColor(c._currentHp, c.getMaxHP()), c._currentHp,
		c.getCurrentExperience(),
		c.statColor(c.getStat(INTELLECT), c.getStat(INTELLECT, true)), c.getStat(INTELLECT),
		c.statColor(c.getStat(LUCK), c.getStat(LUCK, true)), c.getStat(LUCK),
		c.statColor(c._currentSp, c.getMaxSP()), c._currentSp,
		party._gold,
		c.statColor(c.getStat(PERSONALITY), c.getStat(PERSONALITY, true)), c.getStat(PERSONALITY),
		c.statColor(c.getAge(), c.getAge(true)), c.getAge(),
		totalResist,
		party._gems,
		c.statColor(c.getStat(ENDURANCE), c.getStat(ENDURANCE, true)), c.getStat(ENDURANCE),
		c.statColor(c.getCurrentLevel(), c._level._permanent), c.getCurrentLevel(),
		c.getNumSkills(),
		foodVal, (foodVal == 1) ? ' ' : 's',
		c.statColor(c.getStat(SPEED), c.getStat(SPEED, true)), c.getStat(SPEED),
		c.statColor(c.getArmorClass(), c.getArmorClass(true)), c.getArmorClass(),
		c.getNumAwards(),
		Res.CONDITION_COLORS[condition], Res.CONDITION_NAMES[condition],
		condition == NO_CONDITION && party._blessed ? Res.PLUS_14 : "",
		condition == NO_CONDITION && party._powerShield ? Res.PLUS_14 : "",
		condition == NO_CONDITION && party._holyBonus ? Res.PLUS_14 : "",
		condition == NO_CONDITION && party._heroism ? Res.PLUS_14 : ""
	);
}

void CharacterInfo::showCursor(bool flag) {
	const int CURSOR_X[5] = { 9, 60, 111, 176, 0 };
	const int CURSOR_Y[5] = { 23, 46, 69, 92, 115 };

	if (_cursorCell < 20) {
		_iconSprites.draw(0, flag ? 49 : 48,
			Common::Point(CURSOR_X[_cursorCell / 5], CURSOR_Y[_cursorCell % 5]));
	}
}

bool CharacterInfo::expandStat(int attrib, const Character &c) {
	const int STAT_POS[2][20] = {
		{
			61, 61, 61, 61, 61, 112, 112, 112, 112, 112,
			177, 177, 177, 177, 177, 34, 34, 34, 34, 34
		}, {
			24, 47, 70, 93, 116, 24, 47, 70, 93, 116,
			24, 47, 70, 93, 116, 24, 47, 70, 93, 116
		}
	};
	assert(attrib < 20);
	Common::Rect bounds(STAT_POS[0][attrib], STAT_POS[1][attrib],
		STAT_POS[0][attrib] + 143, STAT_POS[1][attrib] + 52);
	Party &party = *_vm->_party;
	Windows &windows = *_vm->_windows;
	uint stat1, stat2;
	uint idx;
	Common::String msg;

	switch (attrib) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		// Basic attributes
		stat1 = c.getStat((Attribute)attrib, false);
		stat2 = c.getStat((Attribute)attrib, true);
		for (idx = 0; idx < ARRAYSIZE(Res.STAT_VALUES) - 1; ++idx)
			if (Res.STAT_VALUES[idx] > (int)stat1)
				break;

		msg = Common::String::format(Res.CURRENT_MAXIMUM_RATING_TEXT, Res.STAT_NAMES[attrib],
			stat1, stat2, Res.RATING_TEXT[idx]);
		break;

	case 7:
		// Age
		stat1 = c.getAge(false);
		stat2 = c.getAge(true);
		msg = Common::String::format(Res.AGE_TEXT, Res.STAT_NAMES[attrib],
			stat1, stat2, c._birthDay, c._birthYear);
		break;

	case 8: {
		// Level
		const int CLASS_ATTACK_GAINS[10] = { 5, 6, 6, 7, 8, 6, 5, 4, 7, 6 };
		idx = c.getCurrentLevel() / CLASS_ATTACK_GAINS[c._class] + 1;

		msg = Common::String::format(Res.LEVEL_TEXT, Res.STAT_NAMES[attrib],
			c.getCurrentLevel(), c._level._permanent,
			idx, idx > 1 ? "s" : "",
			c._level._permanent);
		break;
	}

	case 9:
		// Armor Class
		stat1 = c.getArmorClass(false);
		stat2 = c.getArmorClass(true);
		msg = Common::String::format(Res.CURRENT_MAXIMUM_TEXT, Res.STAT_NAMES[attrib],
			stat1, stat2);
		bounds.setHeight(42);
		break;

	case 10: {
		// Hit Points
		Common::String fmt(Res.CURRENT_MAXIMUM_TEXT);
		const char *p;
		while ((p = strstr(fmt.c_str(), "%u")) != nullptr)
			fmt.setChar('d', p - fmt.c_str() + 1);

		msg = Common::String::format(fmt.c_str(), Res.STAT_NAMES[attrib],
			c._currentHp, c.getMaxHP());
		bounds.setHeight(42);
		break;
	}

	case 11:
		// Spell Points
		stat1 = c._currentSp;
		stat2 = c.getMaxSP();
		msg = Common::String::format(Res.CURRENT_MAXIMUM_TEXT, Res.STAT_NAMES[attrib],
			stat1, stat2);
		bounds.setHeight(42);
		break;

	case 12:
		// Resistences
		msg = Common::String::format(Res.RESISTENCES_TEXT, Res.STAT_NAMES[attrib],
			c._fireResistence._permanent + c.itemScan(11) + c._fireResistence._temporary,
			c._coldResistence._permanent + c.itemScan(13) + c._coldResistence._temporary,
			c._electricityResistence._permanent + c.itemScan(12) + c._electricityResistence._temporary,
			c._poisonResistence._permanent + c.itemScan(14) + c._poisonResistence._temporary,
			c._energyResistence._permanent + c.itemScan(15) + c._energyResistence._temporary,
			c._magicResistence._permanent + c.itemScan(16) + c._magicResistence._temporary);
		bounds.setHeight(80);
		break;

	case 13: {
		// Skills
		Common::String lines[20];
		int numLines = c.getNumSkills();
		if (numLines > 0) {
			for (int skill = THIEVERY; skill <= DANGER_SENSE; ++skill) {
				if (c._skills[skill]) {
					if (skill == THIEVERY) {
						lines[0] = Common::String::format("\n\t020%s%u",
							Res.SKILL_NAMES[THIEVERY], c.getThievery());
					} else {
						lines[skill] = Common::String::format("\n\t020%s", Res.SKILL_NAMES[skill]);
					}
				}
			}
		} else {
			lines[0] = Res.NONE;
			numLines = 1;
		}

		msg = Common::String::format("\x2\x3""c%s\x3l%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
			Res.STAT_NAMES[attrib], lines[0].c_str(), lines[1].c_str(),
			lines[2].c_str(), lines[3].c_str(), lines[4].c_str(), lines[5].c_str(),
			lines[17].c_str(), lines[6].c_str(), lines[7].c_str(), lines[8].c_str(),
			lines[9].c_str(), lines[10].c_str(), lines[11].c_str(), lines[12].c_str(),
			lines[13].c_str(), lines[16].c_str(), lines[14].c_str(), lines[15].c_str());

		bounds.top -= (numLines / 2) * 8;
		bounds.setHeight(numLines * 9 + 26);
		if (bounds.bottom >= SCREEN_HEIGHT)
			bounds.moveTo(bounds.left, SCREEN_HEIGHT - bounds.height() - 1);
		break;
	}

	case 14:
		// Awards
		Awards::show(_vm, &c);
		return false;

	case 15:
		// Experience
		stat1 = c.getCurrentExperience();
		stat2 = c.experienceToNextLevel();
		msg = Common::String::format(Res.EXPERIENCE_TEXT,
			Res.STAT_NAMES[attrib], stat1,
			stat2 == 0 ? Res.ELIGIBLE : Common::String::format("%d", stat2).c_str()
		);
		bounds.setHeight(43);
		break;

	case 16:
		// Gold
		msg = Common::String::format(Res.IN_PARTY_IN_BANK, Res.CONSUMABLE_NAMES[0],
			party._gold, party._bankGold);
		bounds.setHeight(43);
		break;

	case 17:
		// Gems
		msg = Common::String::format(Res.IN_PARTY_IN_BANK, Res.CONSUMABLE_NAMES[1],
			party._gems, party._bankGems);
		bounds.setHeight(43);
		break;

	case 18: {
		// Food
		int food = (party._food / party._activeParty.size()) / 3;
		msg = Common::String::format(Res.FOOD_TEXT, Res.CONSUMABLE_NAMES[2],
			party._food, food, food != 1 ? "s" : "");
		break;
	}

	case 19: {
		// Conditions
		Common::String lines[20];
		int total = 0;
		for (int condition = CURSED; condition <= ERADICATED; ++condition) {
			if (c._conditions[condition]) {
				if (condition >= UNCONSCIOUS) {
					lines[condition] = Common::String::format("\n\t020%s",
						Res.CONDITION_NAMES[condition]);
				} else {
					lines[condition] = Common::String::format("\n\t020%s\t095-%d",
						Res.CONDITION_NAMES[condition], c._conditions[condition]);
				}

				++total;
			}
		}

		Condition condition = c.worstCondition();
		if (condition == NO_CONDITION) {
			lines[0] = Common::String::format("\n\t020%s", Res.GOOD);
			++total;
		}

		if (party._blessed) {
			lines[16] = Common::String::format(Res.BLESSED, party._blessed);
			++total;
		}
		if (party._powerShield) {
			lines[17] = Common::String::format(Res.POWER_SHIELD, party._powerShield);
			++total;
		}
		if (party._holyBonus) {
			lines[18] = Common::String::format(Res.HOLY_BONUS, party._holyBonus);
			++total;
		}
		if (party._heroism) {
			lines[19] = Common::String::format(Res.HEROISM, party._heroism);
			++total;
		}

		msg = Common::String::format("\x2\x3""c%s\x3l%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\x1",
			Res.CONSUMABLE_NAMES[3], lines[0].c_str(), lines[1].c_str(),
			lines[2].c_str(), lines[3].c_str(), lines[4].c_str(),
			lines[5].c_str(), lines[6].c_str(), lines[7].c_str(),
			lines[8].c_str(), lines[9].c_str(), lines[10].c_str(),
			lines[11].c_str(), lines[12].c_str(), lines[13].c_str(),
			lines[14].c_str(), lines[15].c_str(), lines[16].c_str(),
			lines[17].c_str(), lines[18].c_str(), lines[19].c_str()
		);

		bounds.top -= ((total - 1) / 2) * 8;
		bounds.setHeight(total * 9 + 26);
		if (bounds.bottom >= SCREEN_HEIGHT)
			bounds.moveTo(bounds.left, SCREEN_HEIGHT - bounds.height() - 1);
		break;
	}

	default:
		break;
	}

	// Write the data for the stat display
	Window &w = windows[28];
	w.setBounds(bounds);
	w.open();
	w.writeString(msg);
	w.update();

	// Wait for a user key/click
	EventsManager &events = *_vm->_events;
	while (!_vm->shouldExit() && !events.isKeyMousePressed())
		events.pollEventsAndWait();
	events.clearEvents();

	w.close();
	return false;
}

} // End of namespace Xeen
