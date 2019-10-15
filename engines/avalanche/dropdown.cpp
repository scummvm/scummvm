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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* Original name: DROPDOWN		A customized version of Oopmenu (qv). */

#include "avalanche/avalanche.h"
#include "avalanche/dropdown.h"

namespace Avalanche {

void HeadType::init(char trig, char altTrig, Common::String title, byte pos, MenuFunc setupFunc, MenuFunc chooseFunc, DropDownMenu *menu) {
	_trigger = trig;
	_altTrigger = altTrig;
	_title = title;
	_position = pos;
	_xpos = _position * _dropdown->kSpacing + _dropdown->kIndent;
	_xright = (_position + 1) * _dropdown->kSpacing + _dropdown->kIndent;
	_setupFunc = setupFunc;
	_chooseFunc = chooseFunc;

	_dropdown = menu;
}

void HeadType::draw() {
	CursorMan.showMouse(false);
	_dropdown->drawMenuText(_xpos, 1, _trigger, _title, true, false);
	CursorMan.showMouse(true);
}

void HeadType::highlight() {
	CursorMan.showMouse(false);

	_dropdown->_vm->_sound->stopSound();
	_dropdown->drawMenuText(_xpos, 1, _trigger, _title, true, true);

	_dropdown->_activeMenuItem._left = _xpos;
	_dropdown->_activeMenuItem._activeNow = true;
	_dropdown->_activeMenuItem._activeNum = _position;
	_dropdown->_menuActive = true;

	// Force reload and redraw of cursor.
	_dropdown->_vm->_currentMouse = 177;

}

bool HeadType::parseAltTrigger(char key) {
	if (key != _altTrigger)
		return true;
	return false;
}

void MenuItem::init(DropDownMenu *menu) {
	_dropdown = menu;

	_activeNow = false;
	_activeNum = 1;
	_dropdown->_menuActive = false;
}

void MenuItem::reset() {
	_optionNum = 0;
	_width = 0;
	_firstlix = false;
	_oldY = 0;
	_highlightNum = 0;
}

void MenuItem::setupOption(Common::String title, char trigger, Common::String shortcut, bool valid) {
	uint16 width = (title + shortcut).size() + 3;
	if (_width < width)
		_width = width;

	_options[_optionNum]._title = title;
	_options[_optionNum]._trigger = trigger;
	_options[_optionNum]._shortcut = shortcut;
	_options[_optionNum]._valid = valid;
	_optionNum++;
}

void MenuItem::displayOption(byte y, bool highlit) {
	Common::String text = _options[y]._title;
	while (text.size() + _options[y]._shortcut.size() < _width)
		text += ' '; // Pad _options[y] with spaces.
	text += _options[y]._shortcut;

	Color backgroundColor;
	if (highlit)
		backgroundColor = kColorBlack;
	else
		backgroundColor = kColorLightgray;

	_dropdown->_vm->_graphics->drawMenuBlock((_flx1 + 1) * 8, 3 + (y + 1) * 10, (_flx2 + 1) * 8, 13 + (y + 1) * 10, backgroundColor);
	_dropdown->drawMenuText(_left, 4 + (y + 1) * 10, _options[y]._trigger, text, _options[y]._valid, highlit);
}

void MenuItem::display() {
	CursorMan.showMouse(false);

	_firstlix = true;
	_flx1 = _left - 2;
	_flx2 = _left + _width;
	_fly = 15 + _optionNum * 10;
	_activeNow = true;
	_dropdown->_menuActive = true;

	_dropdown->_vm->_graphics->drawMenuItem((_flx1 + 1) * 8, 12, (_flx2 + 1) * 8, _fly);

	displayOption(0, true);
	for (int y = 1; y < _optionNum; y++)
		displayOption(y, false);

	_dropdown->_vm->_currentMouse = 177;

	CursorMan.showMouse(true); // 4 = fletch
}

void MenuItem::wipe() {
	CursorMan.showMouse(false);

	_dropdown->drawMenuText(_dropdown->_menuBar._menuItems[_dropdown->_activeMenuItem._activeNum]._xpos, 1,
		_dropdown->_menuBar._menuItems[_dropdown->_activeMenuItem._activeNum]._trigger,
		_dropdown->_menuBar._menuItems[_dropdown->_activeMenuItem._activeNum]._title, true, false);

	_activeNow = false;
	_dropdown->_menuActive = false;
	_firstlix = false;

	CursorMan.showMouse(true);
}

void MenuItem::moveHighlight(int8 inc) {
	if (inc != 0) {
		int8 highlightNum = _highlightNum + inc;
		if ((highlightNum < 0) || (highlightNum >= _optionNum))
			return;
		_highlightNum = highlightNum;
	}
	CursorMan.showMouse(false);
	displayOption(_oldY, false);
	displayOption(_highlightNum, true);
	_oldY = _highlightNum;
	CursorMan.showMouse(true);
}

/**
 * This makes the menu highlight follow the mouse.
 * @remarks	Originally called 'lightup'
 */
void MenuItem::lightUp(Common::Point cursorPos) {
	if ((cursorPos.x < _flx1 * 8) || (cursorPos.x > _flx2 * 8) || (cursorPos.y <= 25) || (cursorPos.y > ((_fly - 3) * 2 + 1)))
		return;
	_highlightNum = (cursorPos.y - 26) / 20;
	if (_highlightNum == _oldY)
		return;
	moveHighlight(0);
}

void MenuItem::select(byte which) {
	if (!_options[which]._valid)
		return;

	_choiceNum = which;
	wipe();

	if (_choiceNum == _optionNum)
		_choiceNum--; // Off the bottom.
	if (_choiceNum > _optionNum)
		_choiceNum = 0; // Off the top, I suppose.

	(_dropdown->*_dropdown->_menuBar._menuItems[_activeNum]._chooseFunc)();
}

void MenuItem::parseKey(char c) {
	c = toupper(c);
	bool found = false;
	for (int i = 0; i < _optionNum; i++) {
		if ((toupper(_options[i]._trigger) == c) && _options[i]._valid) {
			select(i);
			found = true;
		}
	}
	if (!found)
		_dropdown->_vm->_sound->blip();
}

MenuBar::MenuBar() {
	_menuNum = 0;
	_dropdown = nullptr;
}

void MenuBar::init(DropDownMenu *menu) {
	_dropdown = menu;
	_menuNum = 0;
}

void MenuBar::createMenuItem(char trig, Common::String title, char altTrig, MenuFunc setupFunc, MenuFunc chooseFunc) {
	_menuItems[_menuNum].init(trig, altTrig, title, _menuNum, setupFunc, chooseFunc, _dropdown);
	_menuNum++;
}

void MenuBar::draw() {
	_dropdown->_vm->_graphics->drawMenuBar(kMenuBackgroundColor);

	byte savecp = _dropdown->_vm->_cp;
	_dropdown->_vm->_cp = 3;

	for (int i = 0; i < _menuNum; i++)
		_menuItems[i].draw();

	_dropdown->_vm->_cp = savecp;
}

void MenuBar::parseAltTrigger(char c) {
	byte i = 0;
	while ((i < _menuNum) && (_menuItems[i].parseAltTrigger(c)))
		i++;
	if (i == _menuNum)
		return;
	setupMenuItem(i);
}

void MenuBar::setupMenuItem(byte which) {
	if (_dropdown->_activeMenuItem._activeNow) {
		_dropdown->_activeMenuItem.wipe(); // Get rid of menu.
		if (_dropdown->_activeMenuItem._activeNum == _menuItems[which]._position)
			return; // Clicked on own highlight.
	}
	_menuItems[which].highlight();
	(_dropdown->*_menuItems[which]._setupFunc)();
}

void MenuBar::chooseMenuItem(int16 x) {
	for (int i = 0; i < _menuNum; i++) {
		if ((x > _menuItems[i]._xpos * 8) && (x < _menuItems[i]._xright * 8)) {
			setupMenuItem(i);
			break;
		}
	}
}

DropDownMenu::DropDownMenu(AvalancheEngine *vm) {
	_vm = vm;
	_activeMenuItem.init(this);
	_menuBar.init(this);

	_menuActive = false;
	_lastPerson = kPeopleNone;
}

void DropDownMenu::findWhatYouCanDoWithIt() {
	switch (_vm->_thinks) {
	case kObjectWine:
	case kObjectPotion:
	case kObjectInk:
		_verbStr = Common::String(kVerbCodeExam) + kVerbCodeDrink;
		break;
	case kObjectBell:
		_verbStr = Common::String(kVerbCodeExam) + kVerbCodeRing;
		break;
	case kObjectChastity:
		_verbStr = Common::String(kVerbCodeExam) + kVerbCodeWear;
		break;
	case kObjectLute:
		_verbStr = Common::String(kVerbCodeExam) + kVerbCodePlay;
		break;
	case kObjectMushroom:
	case kObjectOnion:
		_verbStr = Common::String(kVerbCodeExam) + kVerbCodeEat;
		break;
	case kObjectClothes:
		_verbStr = Common::String(kVerbCodeExam) + kVerbCodeWear;
		break;
	default:
		_verbStr = kVerbCodeExam; // Anything else.
	}
}

void DropDownMenu::drawMenuText(int16 x, int16 y, char trigger, Common::String text, bool valid, bool highlighted) {
	Color fontColor;
	Color backgroundColor;
	if (highlighted) {
		fontColor = kColorWhite;
		backgroundColor = kColorBlack;
	} else {
		fontColor = kColorBlack;
		backgroundColor = kColorLightgray;
	}

	byte ander;
	if (valid)
		ander = 255;
	else
		ander = 170;

	FontType font;
	for (uint i = 0; i < text.size(); i++) {
		for (int j = 0; j < 8; j++) {
			byte idx = text[i];
			font[idx][j] = _vm->_font[idx][j] & ander; // Set the font.
			// And set the background of the text to the desired color.
			_vm->_graphics->wipeChar(x * 8 + i * 8, y + j, backgroundColor);
		}
	}

	_vm->_graphics->drawNormalText(text, font, 8, x * 8, y, fontColor);

	// Underline the selected character.
	if ((trigger == 0) || !text.contains(trigger) )
		return;
	else {
		byte i;
		for (i = 0; text[i] != trigger; i++)
			; // Search for the character in the string.

		_vm->_graphics->drawChar(ander, x * 8 + i * 8, y + 8, fontColor);
	}

	_vm->_graphics->refreshScreen();
}

void DropDownMenu::bleep() {
	_vm->_sound->playNote(177, 7);
}

void DropDownMenu::parseKey(char r, char re) {
#if 0
	switch (r) {
	case 0:
	case 224: {
		switch (re) {
		case 'K':
			if (_activeMenuItem._activeNum > 1)  {
				_activeMenuItem.wipe();
				_menuBar.setupMenuItem(_activeMenuItem._activeNum - 1);
			} else {
				// Get menu on the left-hand side.
				_activeMenuItem.wipe();
				_menuBar.chooseMenuItem((_menuBar._menuNum - 1) * kSpacing + kIndent);
			}
			break;
		case 'M':
			if (_activeMenuItem._activeNum < _menuBar._menuNum)  {
				_activeMenuItem.wipe();
				_menuBar.setupMenuItem(_activeMenuItem._activeNum + 1);
			} else {
				// Get menu on the far right-hand side.
				_activeMenuItem.wipe();
				_menuBar.chooseMenuItem(kIndent);
			}
			break;
		case 'H':
			_activeMenuItem.moveHighlight(-1);
			break;
		case 'P':
			_activeMenuItem.moveHighlight(1);
			break;
		default:
			_menuBar.parseAltTrigger(re);
		}
	}
	break;
	case 13:
		_activeMenuItem.select(_activeMenuItem._highlightNum);
		break;
	default:
		if (_activeMenuItem._activeNow)
			_activeMenuItem.parseKey(r);
	}
#endif

	warning("STUB: Dropdown::parseKey()"); // To be implemented properly later! Don't remove the comment above!
}

Common::String DropDownMenu::selectGender(byte x) {
	if (x < 175)
		return "im";
	else
		return "er";
}

void DropDownMenu::setupMenuGame() {
	_activeMenuItem.reset();
	_activeMenuItem.setupOption("Help...", 'H', "f1", true);
	_activeMenuItem.setupOption("Boss Key", 'B', "alt-B", true);
	_activeMenuItem.setupOption("Untrash screen", 'U', "ctrl-f7", false);
	_activeMenuItem.setupOption("Score and rank", 'S', "f9", true);
	_activeMenuItem.setupOption("About Avvy...", 'A', "shift-f10", true);
	_activeMenuItem.display();
}

void DropDownMenu::setupMenuFile() {
	_activeMenuItem.reset();
	_activeMenuItem.setupOption("New game", 'N', "f4", true);
	_activeMenuItem.setupOption("Load...", 'L', "^f3", true);
	_activeMenuItem.setupOption("Save", 'S', "^f2", _vm->_alive);
	_activeMenuItem.setupOption("Save As...", 'v', "", _vm->_alive);
	_activeMenuItem.setupOption("DOS Shell", 'D', "alt-1", false);
	_activeMenuItem.setupOption("Quit", 'Q', "alt-X", true);
	_activeMenuItem.display();
}

void DropDownMenu::setupMenuAction() {
	_activeMenuItem.reset();

	Common::String f5Does = _vm->f5Does();
	for (int i = 0; i < 2; i++)
		if (!f5Does.empty())
			f5Does.deleteChar(0);
	if (f5Does.empty())
		_activeMenuItem.setupOption("Do something", 'D', "f5", false);
	else
		_activeMenuItem.setupOption(f5Does, f5Does[0], "f5", true);
	_activeMenuItem.setupOption("Pause game", 'P', "f6", true);
	if (_vm->_room == kRoomMap)
		_activeMenuItem.setupOption("Journey thither", 'J', "f7", _vm->_animation->nearDoor());
	else
		_activeMenuItem.setupOption("Open the door", 'O', "f7", _vm->_animation->nearDoor());
	_activeMenuItem.setupOption("Look around", 'L', "f8", true);
	_activeMenuItem.setupOption("Inventory", 'I', "Tab", true);
	if (_vm->_animation->_sprites[0]->_speedX == kWalk)
		_activeMenuItem.setupOption("Run fast", 'R', "^R", true);
	else
		_activeMenuItem.setupOption("Walk slowly", 'W', "^W", true);

	_activeMenuItem.display();
}

void DropDownMenu::setupMenuPeople() {
	if (!people.empty())
		people.clear();

	_activeMenuItem.reset();

	for (int i = kPeopleAvalot; i <= kPeopleWisewoman; i++) {
		if (_vm->getRoom((People)i) == _vm->_room) {
			_activeMenuItem.setupOption(_vm->getName((People)i), getNameChar((People)i), "", true);
			people += i;
		}
	}

	_activeMenuItem.display();
}

void DropDownMenu::setupMenuObjects() {
	_activeMenuItem.reset();
	for (int i = 0; i < kObjectNum; i++) {
		if (_vm->_objects[i])
			_activeMenuItem.setupOption(getThing(i + 1), getThingChar(i + 1), "", true);
	}
	_activeMenuItem.display();
}

void DropDownMenu::setupMenuWith() {
	_activeMenuItem.reset();

	if (_vm->_thinkThing) {
		findWhatYouCanDoWithIt();

		for (uint i = 0; i < _verbStr.size(); i++) {
			char vbchar;
			Common::String verb;

			_vm->_parser->verbOpt(_verbStr[i], verb, vbchar);
			_activeMenuItem.setupOption(verb, vbchar, "", true);
		}

		// We disable the "give" option if: (a), you haven't selected anybody, (b), the _person you've selected isn't in the room,
		// or (c), the _person you've selected is YOU!

		if ((_lastPerson == kPeopleAvalot) || (_lastPerson == _vm->_parser->kNothing)
			|| (_vm->getRoom(_lastPerson) != _vm->_room))
			_activeMenuItem.setupOption("Give to...", 'G', "", false); // Not here.
		else {
			_activeMenuItem.setupOption(Common::String("Give to ") + _vm->getName(_lastPerson), 'G', "", true);
			_verbStr = _verbStr + kVerbCodeGive;
		}
	} else {
		_activeMenuItem.setupOption("Examine", 'x', "", true);
		_activeMenuItem.setupOption(Common::String("Talk to h") + selectGender(_vm->_thinks), 'T', "", true);
		_verbStr = Common::String(kVerbCodeExam) + kVerbCodeTalk;
		switch (_vm->_thinks) {
		case kPeopleGeida:
		case kPeopleArkata:
			_activeMenuItem.setupOption("Kiss her", 'K', "", true);
			_verbStr = _verbStr + kVerbCodeKiss;
			break;
		case kPeopleDogfood:
			_activeMenuItem.setupOption("Play his game", 'P', "", !_vm->_wonNim); // True if you HAVEN'T won.
			_verbStr = _verbStr + kVerbCodePlay;
			break;
		case kPeopleMalagauche: {
			bool isSober = !_vm->_teetotal;
			_activeMenuItem.setupOption("Buy some wine", 'w', "", !_vm->_objects[kObjectWine - 1]);
			_activeMenuItem.setupOption("Buy some beer", 'b', "", isSober);
			_activeMenuItem.setupOption("Buy some whisky", 'h', "", isSober);
			_activeMenuItem.setupOption("Buy some cider", 'c', "", isSober);
			_activeMenuItem.setupOption("Buy some mead", 'm', "", isSober);
			_verbStr = _verbStr + 101 + 100 + 102 + 103 + 104;
			}
			break;
		case kPeopleTrader:
			_activeMenuItem.setupOption("Buy an onion", 'o', "", !_vm->_objects[kObjectOnion - 1]);
			_verbStr = _verbStr + 105;
			break;
		default:
			break;
		}
	}
	_activeMenuItem.display();
}

void DropDownMenu::runMenuGame() {
	// Help, boss, untrash screen.
	switch (_activeMenuItem._choiceNum) {
	case 0:
		_vm->callVerb(kVerbCodeHelp);
		break;
	case 1:
		_vm->callVerb(kVerbCodeBoss);
		break;
	case 2:
		_vm->_graphics->refreshScreen();
		break;
	case 3:
		_vm->callVerb(kVerbCodeScore);
		break;
	case 4:
		_vm->callVerb(kVerbCodeInfo);
		break;
	default:
		break;
	}
}

void DropDownMenu::runMenuFile() {
	// New game, load, save, save as, DOS shell, about, quit.
	switch (_activeMenuItem._choiceNum) {
	case 0:
		_vm->callVerb(kVerbCodeRestart);
		break;
	case 1:
		if (!_vm->_parser->_realWords[1].empty())
			_vm->_parser->_realWords[1].clear();
		_vm->callVerb(kVerbCodeLoad);
		break;
	// Case 2 is 'Save', Case 3 is 'Save As'. Both triggers ScummVM save screen.
	case 2:
	case 3:
		if (!_vm->_parser->_realWords[1].empty())
			_vm->_parser->_realWords[1].clear();
		_vm->callVerb(kVerbCodeSave);
		break;
	case 4:
		// Command Prompt, disabled
		break;
	case 5:
		_vm->callVerb(kVerbCodeQuit);
		break;
	default:
		break;
	}
}

void DropDownMenu::runMenuAction() {
	// Get up, pause game, open door, look, inventory, walk/run.
	switch (_activeMenuItem._choiceNum) {
	case 0: {
		_vm->_parser->_person = kPeoplePardon;
		_vm->_parser->_thing = _vm->_parser->kPardon;
		Common::String f5Does = _vm->f5Does();
		VerbCode verb = (VerbCode)(byte)f5Does[0];
		_vm->callVerb(verb);
		}
		break;
	case 1:
		_vm->_parser->_thing = _vm->_parser->kPardon;
		_vm->callVerb(kVerbCodePause);
		break;
	case 2:
		_vm->callVerb(kVerbCodeOpen);
		break;
	case 3:
		_vm->_parser->_thing = _vm->_parser->kPardon;
		_vm->callVerb(kVerbCodeLook);
		break;
	case 4:
		_vm->callVerb(kVerbCodeInv);
		break;
	case 5: {
		AnimationType *avvy = _vm->_animation->_sprites[0];
		if (avvy->_speedX == kWalk)
			avvy->_speedX = kRun;
		else
			avvy->_speedX = kWalk;
		_vm->_animation->updateSpeed();
		}
		break;
	default:
		break;
	}
}

void DropDownMenu::runMenuObjects() {
	_vm->thinkAbout(_vm->_objectList[_activeMenuItem._choiceNum], AvalancheEngine::kThing);
}

void DropDownMenu::runMenuPeople() {
	_vm->thinkAbout(people[_activeMenuItem._choiceNum], AvalancheEngine::kPerson);
	_lastPerson = (People)people[_activeMenuItem._choiceNum];
}

void DropDownMenu::runMenuWith() {
	_vm->_parser->_thing = _vm->_thinks;

	if (_vm->_thinkThing) {
		_vm->_parser->_thing += 49;

		if (_verbStr[_activeMenuItem._choiceNum] == kVerbCodeGive)
			_vm->_parser->_person = _lastPerson;
		else
			_vm->_parser->_person = kPeoplePardon;
	} else {
		switch (_verbStr[_activeMenuItem._choiceNum]) {
		case 100: // Beer
		case 102: // Whisky
		case 103: // Cider
			_vm->_parser->_thing = _verbStr[_activeMenuItem._choiceNum];
			_vm->callVerb(kVerbCodeBuy);
			return;
		case 101: // Wine
			_vm->_parser->_thing = 50;
			_vm->callVerb(kVerbCodeBuy);
			return;
		case 104: // Mead
			_vm->_parser->_thing = 107;
			_vm->callVerb(kVerbCodeBuy);
			return;
		case 105: // Onion (trader)
			_vm->_parser->_thing = 67;
			_vm->callVerb(kVerbCodeBuy);
			return;
		default:
			_vm->_parser->_person = (People)_vm->_parser->_thing;
			_vm->_parser->_thing = Parser::kPardon;
			_vm->_subjectNum = 0;
		}
	}
	_vm->callVerb((VerbCode)(byte)_verbStr[_activeMenuItem._choiceNum]);
}

void DropDownMenu::setup() {
	_menuBar.init(this);
	_activeMenuItem.init(this);

	_menuBar.createMenuItem('F', "File", '!', &Avalanche::DropDownMenu::setupMenuFile, &Avalanche::DropDownMenu::runMenuFile);
	_menuBar.createMenuItem('G', "Game", 34, &Avalanche::DropDownMenu::setupMenuGame, &Avalanche::DropDownMenu::runMenuGame);
	_menuBar.createMenuItem('A', "Action", 30, &Avalanche::DropDownMenu::setupMenuAction, &Avalanche::DropDownMenu::runMenuAction);
	_menuBar.createMenuItem('O', "Objects", 24, &Avalanche::DropDownMenu::setupMenuObjects, &Avalanche::DropDownMenu::runMenuObjects);
	_menuBar.createMenuItem('P', "People", 25, &Avalanche::DropDownMenu::setupMenuPeople, &Avalanche::DropDownMenu::runMenuPeople);
	_menuBar.createMenuItem('W', "With", 17, &Avalanche::DropDownMenu::setupMenuWith, &Avalanche::DropDownMenu::runMenuWith);

	_menuBar.draw();
}

void DropDownMenu::update() {
	_vm->_graphics->saveScreen();

	Common::Point cursorPos = _vm->getMousePos();
	while (!_activeMenuItem._activeNow && (cursorPos.y <= 21) && _vm->_holdLeftMouse) {
		_menuBar.chooseMenuItem(cursorPos.x);
		do {
			_vm->updateEvents();
		} while (_vm->_holdLeftMouse && !_vm->shouldQuit());

		while (!_vm->shouldQuit()) {
			do {
				_vm->updateEvents();

				// We update the cursor's picture.
				cursorPos = _vm->getMousePos();
				// Change arrow...
				if ((0 <= cursorPos.y) && (cursorPos.y <= 21))
					_vm->_graphics->loadMouse(kCurUpArrow); // Up arrow
				else if ((22 <= cursorPos.y) && (cursorPos.y <= 339)) {
					if ((cursorPos.x >= _activeMenuItem._flx1 * 8) && (cursorPos.x <= _activeMenuItem._flx2 * 8) && (cursorPos.y > 21) && (cursorPos.y <= _activeMenuItem._fly * 2 + 1))
						_vm->_graphics->loadMouse(kCurRightArrow); // Right-arrow
					else
						_vm->_graphics->loadMouse(kCurFletch); // Fletch
				} else if ((340 <= cursorPos.y) && (cursorPos.y <= 399))
					_vm->_graphics->loadMouse(kCurScrewDriver); // Screwdriver

				_activeMenuItem.lightUp(cursorPos);

				_vm->_graphics->refreshScreen();
			} while (!_vm->_holdLeftMouse && !_vm->shouldQuit());

			if (_vm->_holdLeftMouse) {
				if (cursorPos.y > 21) {
					if (!((_activeMenuItem._firstlix) && ((cursorPos.x >= _activeMenuItem._flx1 * 8) && (cursorPos.x <= _activeMenuItem._flx2 * 8)
						&& (cursorPos.y >= 24) && (cursorPos.y <= (_activeMenuItem._fly * 2 + 1))))) {
							// Clicked OUTSIDE the menu.
							if (_activeMenuItem._activeNow) {
								_activeMenuItem.wipe();
								_vm->_holdLeftMouse = false;
								_vm->_graphics->removeBackup();
								return;
							} // No "else"- clicking on menu has no effect (only releasing).
						}
				} else {
					// Clicked on menu bar.
					if (_activeMenuItem._activeNow) {
						_activeMenuItem.wipe();
						_vm->_graphics->restoreScreen();

						if (((_activeMenuItem._left * 8) <= cursorPos.x) && (cursorPos.x <= (_activeMenuItem._left * 8 + 80))) { // 80: the width of one menu item on the bar in pixels.
							// If we clicked on the same menu item (the one that is already active) on the bar...
							_vm->_holdLeftMouse = false;
							_vm->_graphics->removeBackup();
							return;
						} else {
							_vm->_holdLeftMouse = true;
							break;
						}
					}
				}

				// NOT clicked button...
				if ((_activeMenuItem._firstlix) && ((cursorPos.x >= _activeMenuItem._flx1 * 8) && (cursorPos.x <= _activeMenuItem._flx2 * 8)
					&& (cursorPos.y >= 12) && (cursorPos.y <= (_activeMenuItem._fly * 2 + 1)))) {

					// We act only if the button is released over a menu item.
					while (!_vm->shouldQuit()) {
						cursorPos = _vm->getMousePos();
						_activeMenuItem.lightUp(cursorPos);
						_vm->_graphics->refreshScreen();

						_vm->updateEvents();
						if (!_vm->_holdLeftMouse)
							break;
					}

					uint16 which = (cursorPos.y - 26) / 20;
					_activeMenuItem.select(which);
					if (_activeMenuItem._options[which]._valid) { // If the menu item wasn't active, we do nothing.
						_vm->_graphics->removeBackup();
						return;
					}
				}
			}
		}
	}

	_vm->_graphics->removeBackup();
}

char DropDownMenu::getThingChar(byte which) {
	static const char thingsChar[] = "WMBParCLguKeSnIohn"; // V=Vinegar

	char result;
	switch (which) {
	case kObjectWine:
		if (_vm->_wineState == 3)
			result = 'V'; // Vinegar
		else
			result = thingsChar[which - 1];
		break;
	default:
		result = thingsChar[which - 1];
	}
	return result;
}

byte DropDownMenu::getNameChar(People whose) {
	static const char ladChar[] = "ASCDMTRwLfgeIyPu";
	static const char lassChar[] = "kG\0xB1o";

	if (whose <= kPeopleJacques)
		return ladChar[whose - kPeopleAvalot];
	else if ((whose >= kPeopleArkata) && (whose <= kPeopleWisewoman))
		return lassChar[whose - kPeopleArkata];
	else
		error("getName() - Unexpected character id %d", (byte) whose);
}

Common::String DropDownMenu::getThing(byte which) {
	static const char things[kObjectNum][20] = {
		"Wine", "Money-bag", "Bodkin", "Potion", "Chastity belt",
		"Crossbow bolt", "Crossbow", "Lute", "Pilgrim's badge", "Mushroom", "Key",
		"Bell", "Scroll", "Pen", "Ink", "Clothes", "Habit", "Onion"
	};

	Common::String result;
	switch (which) {
	case kObjectWine:
		switch (_vm->_wineState) {
		case 1:
		case 4:
			result = Common::String(things[which - 1]);
			break;
		case 3:
			result = "Vinegar";
			break;
		default:
			break;
		}
		break;
	case kObjectOnion:
		if (_vm->_rottenOnion) {
			result = Common::String("rotten onion");
		} else {
			result = Common::String(things[which - 1]);
		}
		break;
	default:
		result = Common::String(things[which - 1]);
		break;
	}
	return result;
}

bool DropDownMenu::isActive() {
	return _menuActive;
}

void DropDownMenu::init() {
	_menuActive = false;
}

void DropDownMenu::resetVariables() {
	_lastPerson = kPeoplePardon;
}
} // End of namespace Avalanche.
