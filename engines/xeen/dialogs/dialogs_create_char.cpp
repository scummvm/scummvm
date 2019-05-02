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

#include "xeen/dialogs/dialogs_create_char.h"
#include "xeen/dialogs/dialogs_input.h"
#include "xeen/xeen.h"

namespace Xeen {

void CreateCharacterDialog::show(XeenEngine *vm) {
	CreateCharacterDialog *dlg = new CreateCharacterDialog(vm);
	dlg->execute();
	delete dlg;
}

CreateCharacterDialog::CreateCharacterDialog(XeenEngine *vm) : ButtonContainer(vm) {
	Common::fill(&_attribs[0], &_attribs[TOTAL_ATTRIBUTES], 0);
	Common::fill(&_allowedClasses[0], &_allowedClasses[TOTAL_CLASSES], false);
	_dicePos[0] = Common::Point(20, 17);
	_dicePos[1] = Common::Point(112, 35);
	_dicePos[2] = Common::Point(61, 50);
	_diceFrame[0] = 0;
	_diceFrame[1] = 2;
	_diceFrame[2] = 4;
	_diceInc[0] = Common::Point(10, -10);
	_diceInc[1] = Common::Point(-10, -10);
	_diceInc[2] = Common::Point(-10, 10);

	_dice.load("dice.vga");
	_diceSize = _dice.getFrameSize(0);

	loadButtons();
}

void CreateCharacterDialog::execute() {
	EventsManager &events = *_vm->_events;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	Windows &windows = *_vm->_windows;
	Window &w = windows[0];
	Common::Array<int> freeCharList;
	int classId = -1;
	int selectedClass = 0;
	bool hasFadedIn = false;
	bool restartFlag = true;
	Race race = HUMAN;
	Sex sex = MALE;
	Common::String msg, details;
	int charIndex = 0;

	Mode oldMode = _vm->_mode;
	_vm->_mode = MODE_4;

	// Load the background
	screen.loadBackground("create.raw");
	events.setCursor(0);

	while (!_vm->shouldExit()) {
		if (restartFlag) {
			// Build up list of roster slot indexes that are free
			freeCharList.clear();
			for (uint idx = 0; idx < XEEN_TOTAL_CHARACTERS; ++idx) {
				if (party._roster[idx]._name.empty())
					freeCharList.push_back(idx);
			}
			charIndex = 0;

			if (freeCharList.size() == XEEN_TOTAL_CHARACTERS)
				break;

			// Get and race and sex for the given character
			race = (Race)((freeCharList[charIndex] / 4) % 5);
			sex = (Sex)(freeCharList[charIndex] & 1);

			// Randomly determine attributes, and which classes they allow
			rollAttributes();

			// Get the display of the rolled character details
			selectedClass = newCharDetails(race, sex, classId, selectedClass, details);
			msg = Common::String::format(Res.CREATE_CHAR_DETAILS,
				details.c_str());

			// Draw the icons and the currently selected headshot
			drawIcons();
			party._roster[freeCharList[charIndex]]._faceSprites->draw(
				w, 0, Common::Point(27, 102));

			// Render all on-screen text
			w.writeString(msg);
			w.update();

			// Draw the arrow for the selected class, if applicable
			if (selectedClass != -1)
				printSelectionArrow(selectedClass);

			// Draw the dice
			drawDice();
			if (!hasFadedIn) {
				screen.fadeIn();
				hasFadedIn = true;
			}

			restartFlag = false;
		}

		// Animate the dice until a user action occurs
		_buttonValue = 0;
		while (!_vm->shouldExit() && !_buttonValue)
			drawDice();

		// Handling for different actions
		if (_buttonValue == Common::KEYCODE_ESCAPE)
			break;

		switch (_buttonValue) {
		case Common::KEYCODE_UP:
			if (charIndex == 0)
				continue;

			--charIndex;
			race = (Race)((freeCharList[charIndex] / 4) % 5);
			sex = (Sex)(freeCharList[charIndex] & 1);
			break;

		case Common::KEYCODE_DOWN:
			if (++charIndex == (int)freeCharList.size()) {
				--charIndex;
				continue;
			} else {
				race = (Race)((freeCharList[charIndex] / 4) % 5);
				sex = (Sex)(freeCharList[charIndex] & 1);
			}
			break;

		case Common::KEYCODE_PAGEUP:
			for (int tempClass = selectedClass - 1; tempClass >= 0; --tempClass) {
				if (_allowedClasses[tempClass]) {
					selectedClass = tempClass;
					break;
				}
			}

			printSelectionArrow(selectedClass);
			continue;

		case Common::KEYCODE_PAGEDOWN:
			break;

		case Common::KEYCODE_m:
		case Common::KEYCODE_i:
		case Common::KEYCODE_p:
		case Common::KEYCODE_e:
		case Common::KEYCODE_s:
		case Common::KEYCODE_a:
		case Common::KEYCODE_l:
			if (swapAttributes(_buttonValue)) {
				checkClass();
				classId = -1;
				selectedClass = newCharDetails(race, sex, classId, selectedClass, msg);
			}
			break;

		case 1000:
		case 1001:
		case 1002:
		case 1003:
		case 1004:
		case 1005:
		case 1006:
		case 1007:
		case 1008:
		case 1009:
			if (_allowedClasses[_buttonValue - 1000]) {
				selectedClass = classId = _buttonValue - 1000;
			}
			break;

		case Common::KEYCODE_c: {
			_vm->_mode = MODE_FF;
			bool result = saveCharacter(party._roster[freeCharList[charIndex]],
				classId, race, sex);
			_vm->_mode = MODE_4;

			if (result)
				restartFlag = true;
			continue;
		}

		case Common::KEYCODE_RETURN:
			classId = selectedClass;
			break;

		case Common::KEYCODE_SPACE:
		case Common::KEYCODE_r:
			// Re-roll the attributes
			rollAttributes();
			classId = -1;
			break;

		default:
			// For all other keypresses, skip the code below the switch
			// statement, and go to wait for the next key
			continue;
		}

		if (_buttonValue != Common::KEYCODE_PAGEDOWN) {
			selectedClass = newCharDetails(race, sex, classId, selectedClass, msg);

			drawIcons2();
			party._roster[freeCharList[charIndex]]._faceSprites->draw(w, 0,
				Common::Point(27, 102));

			w.writeString(msg);
			w.update();

			if (selectedClass != -1) {
				printSelectionArrow(selectedClass);
				continue;
			}
		}

		// Move to next available class, or if the code block above resulted in
		// selectedClass being -1, move to select the first available class
		for (int tempClass = selectedClass + 1; tempClass <= CLASS_RANGER; ++tempClass) {
			if (_allowedClasses[tempClass]) {
				selectedClass = tempClass;
				break;
			}
		}

		printSelectionArrow(selectedClass);
	} while (!_vm->shouldExit() && _buttonValue != Common::KEYCODE_ESCAPE);

	_vm->_mode = oldMode;
}

void CreateCharacterDialog::loadButtons() {
	_icons.load("create.icn");

	// Add buttons
	addButton(Common::Rect(132, 98, 156, 118), Common::KEYCODE_r, &_icons);
	addButton(Common::Rect(132, 128, 156, 148), Common::KEYCODE_c, &_icons);
	addButton(Common::Rect(132, 158, 156, 178), Common::KEYCODE_ESCAPE, &_icons);
	addButton(Common::Rect(86, 98, 110, 118), Common::KEYCODE_UP, &_icons);
	addButton(Common::Rect(86, 120, 110, 140), Common::KEYCODE_DOWN, &_icons);
	addButton(Common::Rect(168, 19, 192, 39), Common::KEYCODE_n, nullptr);
	addButton(Common::Rect(168, 43, 192, 63), Common::KEYCODE_i, nullptr);
	addButton(Common::Rect(168, 67, 192, 87), Common::KEYCODE_p, nullptr);
	addButton(Common::Rect(168, 91, 192, 111), Common::KEYCODE_e, nullptr);
	addButton(Common::Rect(168, 115, 192, 135), Common::KEYCODE_s, nullptr);
	addButton(Common::Rect(168, 139, 192, 159), Common::KEYCODE_a, nullptr);
	addButton(Common::Rect(168, 163, 192, 183), Common::KEYCODE_l, nullptr);
	addButton(Common::Rect(227, 19, 239, 29), 1000, nullptr);
	addButton(Common::Rect(227, 30, 239, 40), 1001, nullptr);
	addButton(Common::Rect(227, 41, 239, 51), 1002, nullptr);
	addButton(Common::Rect(227, 52, 239, 62), 1003, nullptr);
	addButton(Common::Rect(227, 63, 239, 73), 1004, nullptr);
	addButton(Common::Rect(227, 74, 239, 84), 1005, nullptr);
	addButton(Common::Rect(227, 85, 239, 95), 1006, nullptr);
	addButton(Common::Rect(227, 96, 239, 106), 1007, nullptr);
	addButton(Common::Rect(227, 107, 239, 117), 1008, nullptr);
	addButton(Common::Rect(227, 118, 239, 128), 1009, nullptr);
}

void CreateCharacterDialog::drawIcons() {
	// Draw the screen
	_icons.draw(0, 10, Common::Point(168, 19));
	_icons.draw(0, 12, Common::Point(168, 43));
	_icons.draw(0, 14, Common::Point(168, 67));
	_icons.draw(0, 16, Common::Point(168, 91));
	_icons.draw(0, 18, Common::Point(168, 115));
	_icons.draw(0, 20, Common::Point(168, 139));
	_icons.draw(0, 22, Common::Point(168, 163));
	for (int idx = 0; idx < 9; ++idx)
		_icons.draw(0, 24 + idx * 2, Common::Point(227, 19 + 11 * idx));

	for (int idx = 0; idx < 7; ++idx)
		_icons.draw(0, 50 + idx, Common::Point(195, 31 + 24 * idx));

	_icons.draw(0, 57, Common::Point(62, 148));
	_icons.draw(0, 58, Common::Point(62, 158));
	_icons.draw(0, 59, Common::Point(62, 168));
	_icons.draw(0, 61, Common::Point(220, 19));
	_icons.draw(0, 64, Common::Point(220, 155));
	_icons.draw(0, 65, Common::Point(220, 170));

	_icons.draw(0, 0, Common::Point(132, 98));
	_icons.draw(0, 2, Common::Point(132, 128));
	_icons.draw(0, 4, Common::Point(132, 158));
	_icons.draw(0, 6, Common::Point(86, 98));
	_icons.draw(0, 8, Common::Point(86, 120));
}

void CreateCharacterDialog::drawIcons2() {
	for (int idx = 0; idx < 7; ++idx)
		_icons.draw(0, 10 + idx * 2, Common::Point(168, 19 + idx * 24));
	for (int idx = 0; idx < 10; ++idx)
		_icons.draw(0, 24 + idx * 2, Common::Point(227, 19 + idx * 11));
	for (int idx = 0; idx < 8; ++idx)
		_icons.draw(0, 50 + idx, Common::Point(195, 31 + idx * 24));

	_icons.draw(0, 57, Common::Point(62, 148));
	_icons.draw(0, 58, Common::Point(62, 158));
	_icons.draw(0, 59, Common::Point(62, 168));
	_icons.draw(0, 61, Common::Point(220, 19));
	_icons.draw(0, 64, Common::Point(220, 155));
	_icons.draw(0, 65, Common::Point(220, 170));

	_icons.draw(0, 0, Common::Point(132, 98));
	_icons.draw(0, 2, Common::Point(132, 128));
	_icons.draw(0, 4, Common::Point(132, 158));
	_icons.draw(0, 6, Common::Point(86, 98));
	_icons.draw(0, 8, Common::Point(86, 120));
}

void CreateCharacterDialog::rollAttributes() {
	bool repeat = true;
	do {
		// Default all the attributes to zero
		Common::fill(&_attribs[0], &_attribs[TOTAL_ATTRIBUTES], 0);

		// Assign random amounts to each attribute
		for (int idx1 = 0; idx1 < 3; ++idx1) {
			for (int idx2 = 0; idx2 < TOTAL_ATTRIBUTES; ++idx2) {
				_attribs[idx2] += _vm->getRandomNumber(10, 79) / 10;
			}
		}

		// Check which classes are allowed based on the rolled attributes
		checkClass();

		// Only exit if the attributes allow for at least one class
		for (int idx = 0; idx < TOTAL_CLASSES; ++idx) {
			if (_allowedClasses[idx])
				repeat = false;
		}
	} while (repeat);
}

void CreateCharacterDialog::checkClass() {
	_allowedClasses[CLASS_KNIGHT] = _attribs[MIGHT] >= 15;
	_allowedClasses[CLASS_PALADIN] = _attribs[MIGHT] >= 13
		&& _attribs[PERSONALITY] >= 13 && _attribs[ENDURANCE] >= 13;
	_allowedClasses[CLASS_ARCHER] = _attribs[INTELLECT] >= 13 && _attribs[ACCURACY] >= 13;
	_allowedClasses[CLASS_CLERIC] = _attribs[PERSONALITY] >= 13;
	_allowedClasses[CLASS_SORCERER] = _attribs[INTELLECT] >= 13;
	_allowedClasses[CLASS_ROBBER] = _attribs[LUCK] >= 13;
	_allowedClasses[CLASS_NINJA] = _attribs[SPEED] >= 13 && _attribs[ACCURACY] >= 13;
	_allowedClasses[CLASS_BARBARIAN] = _attribs[ENDURANCE] >= 15;
	_allowedClasses[CLASS_DRUID] = _attribs[INTELLECT] >= 15 && _attribs[PERSONALITY] >= 15;
	_allowedClasses[CLASS_RANGER] = _attribs[INTELLECT] >= 12 && _attribs[PERSONALITY] >= 12
		&& _attribs[ENDURANCE] >= 12 && _attribs[SPEED] >= 12;
}

int CreateCharacterDialog::newCharDetails(Race race, Sex sex, int classId,
		int selectedClass, Common::String &msg) {
	int foundClass = -1;
	Common::String skillStr, classStr, raceSkillStr;

	// If a selected class is provided, set the default skill for that class
	if (classId != -1 && Res.NEW_CHAR_SKILLS[classId] != -1) {
		const char *skillP = Res.SKILL_NAMES[Res.NEW_CHAR_SKILLS[classId]];
		skillStr = Common::String(skillP, skillP + Res.NEW_CHAR_SKILLS_LEN[classId]);
	}

	// If a class is provided, set the class name
	if (classId != -1) {
		classStr = Common::String::format("\t062\v168%s", Res.CLASS_NAMES[classId]);
	}

	// Set up default skill for the race, if any
	if (Res.NEW_CHAR_RACE_SKILLS[race] != -1) {
		const char *skillP = Res.SKILL_NAMES[Res.NEW_CHAR_RACE_SKILLS[race]];
		raceSkillStr = Common::String(skillP + Res.NEW_CHAR_SKILLS_OFFSET[race]);
	}

	// Set up color to use for each skill string to be displayed, based
	// on whether each class is allowed or not for the given attributes
	int classColors[TOTAL_CLASSES];
	Common::fill(&classColors[0], &classColors[TOTAL_CLASSES], 0);
	for (int classNum = CLASS_KNIGHT; classNum <= CLASS_RANGER; ++classNum) {
		if (_allowedClasses[classNum]) {
			if (classId == -1 && (foundClass == -1 || foundClass < classNum))
				foundClass = classNum;
			classColors[classNum] = 4;
		}
	}
	if (classId != -1)
		classColors[selectedClass] = 12;

	// Return stats details and character class
	msg = Common::String::format(Res.NEW_CHAR_STATS, Res.RACE_NAMES[race], Res.SEX_NAMES[sex],
		_attribs[MIGHT], _attribs[INTELLECT], _attribs[PERSONALITY],
		_attribs[ENDURANCE], _attribs[SPEED], _attribs[ACCURACY], _attribs[LUCK],
		classColors[CLASS_KNIGHT], classColors[CLASS_PALADIN],
		classColors[CLASS_ARCHER], classColors[CLASS_CLERIC],
		classColors[CLASS_SORCERER], classColors[CLASS_ROBBER],
		classColors[CLASS_NINJA], classColors[CLASS_BARBARIAN],
		classColors[CLASS_DRUID], classColors[CLASS_RANGER],
		skillStr.c_str(), raceSkillStr.c_str(), classStr.c_str()
	);
	return classId == -1 ? foundClass : selectedClass;
}

void CreateCharacterDialog::printSelectionArrow(int selectedClass) {
	Windows &windows = *_vm->_windows;
	Window &w = windows[0];

	_icons.draw(0, 61, Common::Point(220, 19));
	_icons.draw(0, 63, Common::Point(220, selectedClass * 11 + 21));
	w.update();
}

void CreateCharacterDialog::drawDice() {
	EventsManager &events = *_vm->_events;
	Windows &windows = *_vm->_windows;
	Window &w = windows[32];

	// Draw the dice area background
	events.updateGameCounter();
	_dice.draw(w, 7, Common::Point(12, 11));

	// Iterate through each of the three dice
	for (int diceNum = 0; diceNum < 3; ++diceNum) {
		_diceFrame[diceNum] = (_diceFrame[diceNum] + 1) % 7;
		_dicePos[diceNum] += _diceInc[diceNum];

		if (_dicePos[diceNum].x < 13) {
			_dicePos[diceNum].x = 13;
			_diceInc[diceNum].x *= -1;
		} else if (_dicePos[diceNum].x >= (163 - _diceSize.x)) {
			_dicePos[diceNum].x = 163 - _diceSize.x;
			_diceInc[diceNum].x *= -1;
		}

		if (_dicePos[diceNum].y < 12) {
			_dicePos[diceNum].y = 12;
			_diceInc[diceNum].y *= -1;
		} else if (_dicePos[diceNum].y >= (93 - _diceSize.y)) {
			_dicePos[diceNum].y = 93 - _diceSize.y;
			_diceInc[diceNum].y *= -1;
		}

		_dice.draw(w, _diceFrame[diceNum], _dicePos[diceNum]);
	}

	// Wait for a single frame, checking for any events
	w.update();
	events.wait(1);
	checkEvents(_vm);
}

int CreateCharacterDialog::getAttribFromKeycode(int keycode) const {
	switch (keycode) {
	case Common::KEYCODE_m:
		return MIGHT;
	case Common::KEYCODE_i:
		return INTELLECT;
	case Common::KEYCODE_p:
		return PERSONALITY;
	case Common::KEYCODE_e:
		return ENDURANCE;
	case Common::KEYCODE_s:
		return SPEED;
	case Common::KEYCODE_a:
		return ACCURACY;
	case Common::KEYCODE_l:
		return LUCK;
	default:
		return -1;
	}
}

bool CreateCharacterDialog::swapAttributes(int keycode) {
	Windows &windows = *_vm->_windows;
	Window &w = windows[0];

	int srcAttrib = getAttribFromKeycode(keycode);
	assert(srcAttrib >= 0);

	_vm->_mode = MODE_86;
	_icons.draw(w, srcAttrib * 2 + 11, Common::Point(
		_buttons[srcAttrib + 5]._bounds.left, _buttons[srcAttrib + 5]._bounds.top));
	w.update();

	int destAttrib = exchangeAttribute(srcAttrib);
	if (destAttrib != -1) {
		_icons.draw(w, destAttrib * 2 + 11, Common::Point(
			_buttons[destAttrib + 5]._bounds.left,
			_buttons[destAttrib + 5]._bounds.top));

		SWAP(_attribs[srcAttrib], _attribs[destAttrib]);
		return true;

	} else {
		_icons.draw(w, srcAttrib * 2 + 10, Common::Point(
			_buttons[srcAttrib + 5]._bounds.left,
			_buttons[srcAttrib + 5]._bounds.top));
		w.update();
		_vm->_mode = MODE_SLEEPING;
		return false;
	}
}

int CreateCharacterDialog::exchangeAttribute(int srcAttr) {
	EventsManager &events = *_vm->_events;
	Windows &windows = *_vm->_windows;
	SpriteResource icons;
	icons.load("create2.icn");

	saveButtons();
	addButton(Common::Rect(118, 58, 142, 78), Common::KEYCODE_ESCAPE, &_icons);
	addButton(Common::Rect(168, 19, 192, 39), Common::KEYCODE_m);
	addButton(Common::Rect(168, 43, 192, 63), Common::KEYCODE_i);
	addButton(Common::Rect(168, 67, 192, 87), Common::KEYCODE_p);
	addButton(Common::Rect(168, 91, 192, 111), Common::KEYCODE_e);
	addButton(Common::Rect(168, 115, 192, 135), Common::KEYCODE_s);
	addButton(Common::Rect(168, 139, 192, 159), Common::KEYCODE_a);
	addButton(Common::Rect(168, 163, 192, 183), Common::KEYCODE_l);

	Window &w = windows[26];
	w.open();
	w.writeString(Common::String::format(Res.EXCHANGE_ATTR_WITH, Res.STAT_NAMES[srcAttr]));
	icons.draw(w, 0, Common::Point(118, 58));
	w.update();

	int result = -1;
	bool breakFlag = false;
	while (!_vm->shouldExit() && !breakFlag) {
		// Wait for an action
		do {
			events.pollEventsAndWait();
			checkEvents(_vm);
		} while (!_vm->shouldExit() && !_buttonValue);
		if (_buttonValue == Common::KEYCODE_ESCAPE)
			break;

		int destAttr = getAttribFromKeycode(_buttonValue);

		if (destAttr != -1 && srcAttr != destAttr) {
			result = destAttr;
			break;
		}
	}

	w.close();
	restoreButtons();
	_buttonValue = 0;
	return result;
}

bool CreateCharacterDialog::saveCharacter(Character &c, int classId, Race race, Sex sex) {
	if (classId == -1) {
		ErrorScroll::show(_vm, Res.SELECT_CLASS_BEFORE_SAVING);
		return false;
	}

	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Windows &windows = *_vm->_windows;
	Window &w = windows[6];
	Common::String name;
	int result;
	int ccNum = _vm->_files->_ccNum;

	// Prompt for a character name
	w.open();
	w.writeString(Res.NAME_FOR_NEW_CHARACTER);
	saveButtons();
	result = Input::show(_vm, &w, name, 10, 200);
	restoreButtons();
	w.close();

	if (!result)
		// Name aborted, so exit
		return false;

	// Save new character details
	c.clear();
	c._name = name;
	c._savedMazeId = party._priorMazeId;
	c._xeenSide = map._loadCcNum;
	c._sex = sex;
	c._race = race;
	c._class = (CharacterClass)classId;
	c._level._permanent = ccNum ? 5 : 1;

	c._might._permanent = _attribs[MIGHT];
	c._intellect._permanent = _attribs[INTELLECT];
	c._personality._permanent = _attribs[PERSONALITY];
	c._endurance._permanent = _attribs[ENDURANCE];
	c._speed._permanent = _attribs[SPEED];
	c._accuracy._permanent = _attribs[ACCURACY];
	c._luck._permanent = _attribs[LUCK];

	c._magicResistence._permanent = Res.RACE_MAGIC_RESISTENCES[race];
	c._fireResistence._permanent = Res.RACE_FIRE_RESISTENCES[race];
	c._electricityResistence._permanent = Res.RACE_ELECTRIC_RESISTENCES[race];
	c._coldResistence._permanent = Res.RACE_COLD_RESISTENCES[race];
	c._energyResistence._permanent = Res.RACE_ENERGY_RESISTENCES[race];
	c._poisonResistence._permanent = Res.RACE_POISON_RESISTENCES[race];

	c._birthYear = party._year - 18;
	c._birthDay = party._day;
	c._hasSpells = false;
	c._currentSpell = -1;

	// Set up any default spells for the character's class
	for (int idx = 0; idx < 4; ++idx) {
		if (Res.NEW_CHARACTER_SPELLS[c._class][idx] != -1) {
			c._hasSpells = true;
			c._currentSpell = Res.NEW_CHARACTER_SPELLS[c._class][idx];
			c._spells[c._currentSpell] = true;
		}
	}

	int classSkill = Res.NEW_CHAR_SKILLS[c._class];
	if (classSkill != -1)
		c._skills[classSkill] = 1;

	int raceSkill = Res.NEW_CHAR_RACE_SKILLS[c._race];
	if (raceSkill != -1)
		c._skills[raceSkill] = 1;

	c._currentHp = c.getMaxHP();
	c._currentSp = c.getMaxSP();
	return true;
}

} // End of namespace Xeen
