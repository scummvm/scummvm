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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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
#include "avalanche/menu.h"
#include "avalanche/lucerna.h"
#include "avalanche/gyro.h"
#include "avalanche/acci.h"
#include "avalanche/animation.h"

#include "common/textconsole.h"

namespace Avalanche {

void HeadType::init(char trig, char altTrig, Common::String title, byte pos, MenuFunc setupFunc, MenuFunc chooseFunc, Menu *dr) {
	_trigger = trig;
	_altTrigger = altTrig;
	_title = title;
	_position = pos;
	_xpos = _position * _dr->kSpacing + _dr->kIndent;
	_xright = (_position + 1) * _dr->kSpacing + _dr->kIndent;
	_setupFunc = setupFunc;
	_chooseFunc = chooseFunc;

	_dr = dr;
}

void HeadType::draw() {
	CursorMan.showMouse(false);
	_dr->drawMenuText(_xpos, 1, _trigger, _title, true, false);
	CursorMan.showMouse(true);
}

void HeadType::highlight() {
	CursorMan.showMouse(false);

	_dr->_vm->_sound->stopSound();
	_dr->drawMenuText(_xpos, 1, _trigger, _title, true, true);

	_dr->_activeMenuItem._left = _xpos;
	_dr->_activeMenuItem._activeNow = true;
	_dr->_activeMenuItem._activeNum = _position;
	_dr->_menuActive = true;

	_dr->_vm->_gyro->_currentMouse = 177; // Force redraw of cursor.
}

bool HeadType::parseAltTrigger(char key) {
	if (key != _altTrigger)
		return true;
	return false;
}

void MenuItem::init(Menu *dr) {
	_dr = dr;

	_activeNow = false;
	_activeNum = 1;
	_dr->_menuActive = false;
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
	byte backgroundColor;
	if (highlit)
		backgroundColor = 0;
	else
		backgroundColor = 7;
	_dr->_vm->_graphics->_surface.fillRect(Common::Rect((_flx1 + 1) * 8, 3 + (y + 1) * 10, (_flx2 + 1) * 8, 13 + (y + 1) * 10), backgroundColor);

	Common::String text = _options[y]._title;
	while (text.size() + _options[y]._shortcut.size() < _width)
		text += ' '; // Pad _options[y] with spaces.
	text += _options[y]._shortcut;

	_dr->drawMenuText(_left, 4 + (y + 1) * 10, _options[y]._trigger, text, _options[y]._valid, highlit);
}

void MenuItem::display() {
	CursorMan.showMouse(false);
	//setfillstyle(1, menu_b);
	//setcolor(menu_border);
	_firstlix = true;
	_flx1 = _left - 2;
	_flx2 = _left + _width;
	_fly = 15 + _optionNum * 10;
	_activeNow = true;
	_dr->_menuActive = true;

	_dr->_vm->_graphics->_surface.fillRect(Common::Rect((_flx1 + 1) * 8, 12, (_flx2 + 1) * 8, _fly), _dr->kMenuBackgroundColor);
	_dr->_vm->_graphics->_surface.frameRect(Common::Rect((_flx1 + 1) * 8 - 1, 11, (_flx2 + 1) * 8 + 1, _fly + 1), _dr->kMenuBorderColor);

	displayOption(0, true);
	for (int y = 1; y < _optionNum; y++)
		displayOption(y, false);

	_dr->_vm->_gyro->_defaultLed = 1;
	_dr->_vm->_gyro->_currentMouse = 177;

	CursorMan.showMouse(true); // 4 = fletch
}

void MenuItem::wipe() {
	CursorMan.showMouse(false);

	_dr->drawMenuText(_dr->_menuBar._menuItems[_dr->_activeMenuItem._activeNum]._xpos, 1, _dr->_menuBar._menuItems[_dr->_activeMenuItem._activeNum]._trigger, _dr->_menuBar._menuItems[_dr->_activeMenuItem._activeNum]._title, true, false);

	_activeNow = false;
	_dr->_menuActive = false;
	_firstlix = false;
	_dr->_vm->_gyro->_defaultLed = 2;

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

	(_dr->*_dr->_menuBar._menuItems[_activeNum]._chooseFunc)();
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
		_dr->_vm->_sound->blip();
}

void MenuBar::init(Menu *dr) {
	_dr = dr;
	_menuNum = 0;
}

void MenuBar::createMenuItem(char trig, Common::String title, char altTrig, MenuFunc setupFunc, MenuFunc chooseFunc) {
	_menuItems[_menuNum].init(trig, altTrig, title, _menuNum, setupFunc, chooseFunc, _dr);
	_menuNum++;
}

void MenuBar::draw() {
	_dr->_vm->_graphics->_surface.fillRect(Common::Rect(0, 0, 640, 10), _dr->kMenuBackgroundColor);

	byte savecp = _dr->_vm->_gyro->_cp;
	_dr->_vm->_gyro->_cp = 3;

	for (int i = 0; i < _menuNum; i++)
		_menuItems[i].draw();

	_dr->_vm->_gyro->_cp = savecp;
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
	if (_dr->_activeMenuItem._activeNow) {
		_dr->_activeMenuItem.wipe(); // Get rid of menu.
		if (_dr->_activeMenuItem._activeNum == _menuItems[which]._position)
			return; // Clicked on own highlight.
	}
	_menuItems[which].highlight();
	(_dr->*_menuItems[which]._setupFunc)();
}

void MenuBar::chooseMenuItem(int16 x) {
	for (int i = 0; i < _menuNum; i++) {
		if ((x > _menuItems[i]._xpos * 8) && (x < _menuItems[i]._xright * 8)) {
			setupMenuItem(i);
			break;
		}
	}
}

Menu::Menu(AvalancheEngine *vm) {
	_vm = vm;
	_activeMenuItem.init(this);
	_menuBar.init(this);
}

void Menu::findWhatYouCanDoWithIt() {
	switch (_vm->_gyro->_thinks) {
	case Gyro::kObjectWine:
	case Gyro::kObjectPotion:
	case Gyro::kObjectInk:
		_vm->_gyro->_verbStr = Common::String(Acci::kVerbCodeExam) + Acci::kVerbCodeDrink;
		break;
	case Gyro::kObjectBell:
		_vm->_gyro->_verbStr = Common::String(Acci::kVerbCodeExam) + Acci::kVerbCodeRing;
		break;
	case Gyro::kObjectChastity:
		_vm->_gyro->_verbStr = Common::String(Acci::kVerbCodeExam) + Acci::kVerbCodeWear;
		break;
	case Gyro::kObjectLute:
		_vm->_gyro->_verbStr = Common::String(Acci::kVerbCodeExam) + Acci::kVerbCodePlay;
		break;
	case Gyro::kObjectMushroom:
	case Gyro::kObjectOnion:
		_vm->_gyro->_verbStr = Common::String(Acci::kVerbCodeExam) + Acci::kVerbCodeEat;
		break;
	case Gyro::kObjectClothes:
		_vm->_gyro->_verbStr = Common::String(Acci::kVerbCodeExam) + Acci::kVerbCodeWear;
		break;
	default:
		_vm->_gyro->_verbStr = Acci::kVerbCodeExam; // Anything else.
	}
}

void Menu::drawMenuText(int16 x, int16 y, char trigger, Common::String text, bool valid, bool highlighted) {
	byte fontColor;
	byte backgroundColor;
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
			font[idx][j] = _vm->_gyro->_font[idx][j] & ander; // Set the font.
			// And set the background of the text to the desired color.
			for (int k = 0; k < 8; k++)
				*(byte *)_vm->_graphics->_surface.getBasePtr(x * 8 + i * 8 + k, y + j) = backgroundColor;
		}
	}

	_vm->_graphics->drawText(_vm->_graphics->_surface, text, font, 8, x * 8, y, fontColor);

	// Underline the selected character.
	if ((trigger == 0) || !text.contains(trigger) )
		return;
	else {
		byte i;
		for (i = 0; text[i] != trigger; i++)
			; // Search for the character in the string.

		byte pixel = ander;
		for (int bit = 0; bit < 8; bit++) {
			byte pixelBit = (pixel >> bit) & 1;
			if (pixelBit)
				*(byte *)_vm->_graphics->_surface.getBasePtr(x * 8 + i * 8 + 7 - bit, y + 8) = fontColor;
		}
	}

	_vm->_graphics->refreshScreen();
}

void Menu::bleep() {
	warning("STUB: Dropdown::bleep()");
}

void Menu::parseKey(char r, char re) {
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

Common::String Menu::selectGender(byte x) {
	if (x < 175)
		return "im";
	else
		return "er";
}

void Menu::setupMenuGame() {
	_activeMenuItem.reset();
	_activeMenuItem.setupOption("Help...", 'H', "f1", true);
	_activeMenuItem.setupOption("Boss Key", 'B', "alt-B", true);
	_activeMenuItem.setupOption("Untrash screen", 'U', "ctrl-f7", true);
	_activeMenuItem.setupOption("Score and rank", 'S', "f9", true);
	_activeMenuItem.setupOption("About Avvy...", 'A', "shift-f10", true);
	_activeMenuItem.display();
}

void Menu::setupMenuFile() {
	_activeMenuItem.reset();
	_activeMenuItem.setupOption("New game", 'N', "f4", true);
	_activeMenuItem.setupOption("Load...", 'L', "^f3", true);
	_activeMenuItem.setupOption("Save", 'S', "^f2", _vm->_gyro->_alive);
	_activeMenuItem.setupOption("Save As...", 'v', "", _vm->_gyro->_alive);
	_activeMenuItem.setupOption("DOS Shell", 'D', _vm->_gyro->_atKey + '1', true);
	_activeMenuItem.setupOption("Quit", 'Q', "alt-X", true);
	_activeMenuItem.display();
}

void Menu::setupMenuAction() {
	_activeMenuItem.reset();

	Common::String f5Does = _vm->_gyro->f5Does();
	for (int i = 0; i < 2; i++)
		if (!f5Does.empty())
			f5Does.deleteChar(0);
	if (f5Does.empty())
		_activeMenuItem.setupOption("Do something", 'D', "f5", false);
	else
		_activeMenuItem.setupOption(f5Does, f5Does[0], "f5", true);
	_activeMenuItem.setupOption("Pause game", 'P', "f6", true);
	if (_vm->_gyro->_room == kRoomMap)
		_activeMenuItem.setupOption("Journey thither", 'J', "f7", _vm->_animation->nearDoor());
	else
		_activeMenuItem.setupOption("Open the door", 'O', "f7", _vm->_animation->nearDoor());
	_activeMenuItem.setupOption("Look around", 'L', "f8", true);
	_activeMenuItem.setupOption("Inventory", 'I', "Tab", true);
	if (_vm->_animation->_sprites[0]._speedX == _vm->_gyro->kWalk)
		_activeMenuItem.setupOption("Run fast", 'R', "^R", true);
	else
		_activeMenuItem.setupOption("Walk slowly", 'W', "^W", true);

	_activeMenuItem.display();
}

void Menu::setupMenuPeople() {
	if (!people.empty())
		people.clear();

	_activeMenuItem.reset();

	for (int i = 150; i <= 178; i++) {
		if (_vm->_gyro->_whereIs[i - 150] == _vm->_gyro->_room) {
			_activeMenuItem.setupOption(_vm->_gyro->getName(i), _vm->_gyro->getNameChar(i), "", true);
			people += i;
		}
	}

	_activeMenuItem.display();
}

void Menu::setupMenuObjects() {
	_activeMenuItem.reset();
	for (int i = 0; i < kObjectNum; i++) {
		if (_vm->_gyro->_objects[i])
			_activeMenuItem.setupOption(_vm->_gyro->getThing(i + 1), _vm->_gyro->getThingChar(i + 1), "", true);
	}
	_activeMenuItem.display();
}

void Menu::setupMenuWith() {
	_activeMenuItem.reset();

	if (_vm->_gyro->_thinkThing) {
		findWhatYouCanDoWithIt();

		for (uint i = 0; i < _vm->_gyro->_verbStr.size(); i++) {
			char vbchar;
			Common::String verb;

			_vm->_acci->verbOpt(_vm->_gyro->_verbStr[i], verb, vbchar);
			_activeMenuItem.setupOption(verb, vbchar, "", true);
		}

		// We disable the "give" option if: (a), you haven't selected anybody, (b), the _person you've selected isn't in the room,
		// or (c), the _person you've selected is YOU!

		if ((_vm->_gyro->_lastPerson == Gyro::kPeopleAvalot) || (_vm->_gyro->_lastPerson == _vm->_acci->kNothing)
			|| (_vm->_gyro->_whereIs[_vm->_gyro->_lastPerson - 150] != _vm->_gyro->_room))
			_activeMenuItem.setupOption("Give to...", 'G', "", false); // Not here.
		else {
			_activeMenuItem.setupOption(Common::String("Give to ") + _vm->_gyro->getName(_vm->_gyro->_lastPerson), 'G', "", true);
			_vm->_gyro->_verbStr = _vm->_gyro->_verbStr + Acci::kVerbCodeGive;
		}
	} else {
		_activeMenuItem.setupOption("Examine", 'x', "", true);
		_activeMenuItem.setupOption(Common::String("Talk to h") + selectGender(_vm->_gyro->_thinks), 'T', "", true);
		_vm->_gyro->_verbStr = Common::String(Acci::kVerbCodeExam) + Acci::kVerbCodeTalk;
		switch (_vm->_gyro->_thinks) {
		case Gyro::kPeopleGeida:
		case Gyro::kPeopleArkata:
			_activeMenuItem.setupOption("Kiss her", 'K', "", true);
			_vm->_gyro->_verbStr = _vm->_gyro->_verbStr + Acci::kVerbCodeKiss;
			break;
		case Gyro::kPeopleDogfood:
			_activeMenuItem.setupOption("Play his game", 'P', "", !_vm->_gyro->_wonNim); // True if you HAVEN'T won.
			_vm->_gyro->_verbStr = _vm->_gyro->_verbStr + Acci::kVerbCodePlay;
			break;
		case Gyro::kPeopleMalagauche: {
			bool isSober = !_vm->_gyro->_teetotal;
			_activeMenuItem.setupOption("Buy some wine", 'w', "", !_vm->_gyro->_objects[Gyro::kObjectWine - 1]);
			_activeMenuItem.setupOption("Buy some beer", 'b', "", isSober);
			_activeMenuItem.setupOption("Buy some whisky", 'h', "", isSober);
			_activeMenuItem.setupOption("Buy some cider", 'c', "", isSober);
			_activeMenuItem.setupOption("Buy some mead", 'm', "", isSober);
			_vm->_gyro->_verbStr = _vm->_gyro->_verbStr + 101 + 100 + 102 + 103 + 104;
			}
			break;
		case Gyro::kPeopleTrader:
			_activeMenuItem.setupOption("Buy an onion", 'o', "", !_vm->_gyro->_objects[Gyro::kObjectOnion - 1]);
			_vm->_gyro->_verbStr = _vm->_gyro->_verbStr + 105;
			break;
		}
	}
	_activeMenuItem.display();
}

void Menu::runMenuGame() {
	// Help, boss, untrash screen.
	switch (_activeMenuItem._choiceNum) {
	case 0:
		_vm->_lucerna->callVerb(Acci::kVerbCodeHelp);
		break;
	case 1:
		_vm->_lucerna->callVerb(Acci::kVerbCodeBoss);
		break;
	case 2:
		_vm->_lucerna->majorRedraw();
		break;
	case 3:
		_vm->_lucerna->callVerb(Acci::kVerbCodeScore);
		break;
	case 4:
		_vm->_lucerna->callVerb(Acci::kVerbCodeInfo);
		break;
	}
}

void Menu::runMenuFile() {
	// New game, load, save, save as, DOS shell, about, quit.
	switch (_activeMenuItem._choiceNum) {
	case 0:
		_vm->_lucerna->callVerb(Acci::kVerbCodeRestart);
		break;
	case 1:
		if (!_vm->_acci->_realWords[1].empty())
			_vm->_acci->_realWords[1].clear();
		_vm->_lucerna->callVerb(Acci::kVerbCodeLoad);
		break;
	case 2:
		if (!_vm->_acci->_realWords[1].empty())
			_vm->_acci->_realWords[1].clear();
		_vm->_lucerna->callVerb(Acci::kVerbCodeSave);
		break;
	case 3:
		//_vm->_basher->filename_edit();
		warning("STUB: Dropdown::runMenuFile()");
		break;
	case 4:
		//_vm->_enid->backToBootstrap(2); TODO: Replace it with proper ScummVM-friendly function(s)!  Do not remove until then!
		break;
	case 5:
		_vm->_lucerna->callVerb(Acci::kVerbCodeQuit);
		break;
	}
}

void Menu::runMenuAction() {
	Common::String f5Does;
	// Get up, pause game, open door, look, inventory, walk/run.
	switch (_activeMenuItem._choiceNum) {
	case 0:
		_vm->_acci->_person = _vm->_acci->kPardon;
		_vm->_acci->_thing = _vm->_acci->kPardon;
		f5Does = _vm->_gyro->f5Does();
		_vm->_lucerna->callVerb(f5Does[0]);
		break;
	case 1:
		_vm->_acci->_thing = _vm->_acci->kPardon;
		_vm->_lucerna->callVerb(Acci::kVerbCodePause);
		break;
	case 2:
		_vm->_lucerna->callVerb(Acci::kVerbCodeOpen);
		break;
	case 3:
		_vm->_acci->_thing = _vm->_acci->kPardon;
		_vm->_lucerna->callVerb(Acci::kVerbCodeLook);
		break;
	case 4:
		_vm->_lucerna->callVerb(Acci::kVerbCodeInv);
		break;
	case 5: {
		AnimationType *avvy = &_vm->_animation->_sprites[0];
		if (avvy->_speedX == _vm->_gyro->kWalk)
			avvy->_speedX = _vm->_gyro->kRun;
		else
			avvy->_speedX = _vm->_gyro->kWalk;
		_vm->_animation->updateSpeed();
		}
		break;
	}
}

void Menu::runMenuObjects() {
	_vm->_lucerna->thinkAbout(_vm->_gyro->_objectList[_activeMenuItem._choiceNum], Gyro::kThing);
}

void Menu::runMenuPeople() {
	_vm->_lucerna->thinkAbout(people[_activeMenuItem._choiceNum], Gyro::kPerson);
	_vm->_gyro->_lastPerson = people[_activeMenuItem._choiceNum];
}

void Menu::runMenuWith() {
	_vm->_acci->_thing = _vm->_gyro->_thinks;

	if (_vm->_gyro->_thinkThing) {
		_vm->_acci->_thing += 49;

		if (_vm->_gyro->_verbStr[_activeMenuItem._choiceNum] == Acci::kVerbCodeGive)
			_vm->_acci->_person = _vm->_gyro->_lastPerson;
		else
			_vm->_acci->_person = Acci::kPardon;
	} else {
		switch (_vm->_gyro->_verbStr[_activeMenuItem._choiceNum]) {
		case 100: // Beer
			_vm->_acci->_thing = 100;
			_vm->_lucerna->callVerb(Acci::kVerbCodeBuy);
			return;
		case 101: // Wine
			_vm->_acci->_thing = 50;
			_vm->_lucerna->callVerb(Acci::kVerbCodeBuy);
			return;
		case 102: // Whisky
			_vm->_acci->_thing = 102;
			_vm->_lucerna->callVerb(Acci::kVerbCodeBuy);
			return;
		case 103: // Cider
			_vm->_acci->_thing = 103;
			_vm->_lucerna->callVerb(Acci::kVerbCodeBuy);
			return;
		case 104: // Mead
			_vm->_acci->_thing = 107;
			_vm->_lucerna->callVerb(Acci::kVerbCodeBuy);
			return;
		case 105: // Onion (trader)
			_vm->_acci->_thing = 67;
			_vm->_lucerna->callVerb(Acci::kVerbCodeBuy);
			return;
		default:
			_vm->_acci->_person = _vm->_acci->_thing;
			_vm->_acci->_thing = Acci::kPardon;
			_vm->_gyro->_subjectNum = 0;
		}
	}
	_vm->_lucerna->callVerb(_vm->_gyro->_verbStr[_activeMenuItem._choiceNum]);
}

void Menu::setup() {
	_menuBar.init(this);
	_activeMenuItem.init(this);

	_menuBar.createMenuItem('F', "File", '!', &Avalanche::Menu::setupMenuFile, &Avalanche::Menu::runMenuFile);
	_menuBar.createMenuItem('G', "Game", 34, &Avalanche::Menu::setupMenuGame, &Avalanche::Menu::runMenuGame);
	_menuBar.createMenuItem('A', "Action", 30, &Avalanche::Menu::setupMenuAction, &Avalanche::Menu::runMenuAction);
	_menuBar.createMenuItem('O', "Objects", 24, &Avalanche::Menu::setupMenuObjects, &Avalanche::Menu::runMenuObjects);
	_menuBar.createMenuItem('P', "People", 25, &Avalanche::Menu::setupMenuPeople, &Avalanche::Menu::runMenuPeople);
	_menuBar.createMenuItem('W', "With", 17, &Avalanche::Menu::setupMenuWith, &Avalanche::Menu::runMenuWith);

	_menuBar.draw();
}

void Menu::update() { // TODO: Optimize it ASAP!!! It really needs it...
	Common::Point cursorPos = _vm->getMousePos();
	::Graphics::Surface backup;
	backup.copyFrom(_vm->_graphics->_surface);

	while (!_activeMenuItem._activeNow && (cursorPos.y <= 21) && _vm->_lucerna->_holdLeftMouse) {
		_menuBar.chooseMenuItem(cursorPos.x);
		do
			_vm->updateEvents();
		while (_vm->_lucerna->_holdLeftMouse);

		while (!_vm->shouldQuit()) {
			do {
				_vm->updateEvents();

				// We updadte the cursor's picture.
				cursorPos = _vm->getMousePos();
				// Change arrow...
				if ((0 <= cursorPos.y) && (cursorPos.y <= 21))
					_vm->_gyro->newMouse(0); // Up arrow
				else if ((22 <= cursorPos.y) && (cursorPos.y <= 339)) {
					if ((cursorPos.x >= _activeMenuItem._flx1 * 8) && (cursorPos.x <= _activeMenuItem._flx2 * 8) && (cursorPos.y > 21) && (cursorPos.y <= _activeMenuItem._fly * 2 + 1))
						_vm->_gyro->newMouse(2); // Right-arrow
					else
						_vm->_gyro->newMouse(3); // Fletch
				} else if ((340 <= cursorPos.y) && (cursorPos.y <= 399))
					_vm->_gyro->newMouse(1); // Screwdriver

				_activeMenuItem.lightUp(cursorPos);

				_vm->_graphics->refreshScreen();
			} while (!_vm->_lucerna->_holdLeftMouse);

			if (_vm->_lucerna->_holdLeftMouse) {
				if (cursorPos.y > 21) {
					if (!((_activeMenuItem._firstlix) && ((cursorPos.x >= _activeMenuItem._flx1 * 8) && (cursorPos.x <= _activeMenuItem._flx2 * 8)
						&& (cursorPos.y >= 24) && (cursorPos.y <= (_activeMenuItem._fly * 2 + 1))))) {
							// Clicked OUTSIDE the menu.
							if (_activeMenuItem._activeNow) {
								_activeMenuItem.wipe();
								_vm->_lucerna->_holdLeftMouse = false;
								backup.free();
								return;
							} // No "else"- clicking on menu has no effect (only releasing).
						}
				} else {
					// Clicked on menu bar.
					if (_activeMenuItem._activeNow) {
						_activeMenuItem.wipe();
						_vm->_graphics->_surface.copyFrom(backup);
						_vm->_graphics->refreshScreen();

						if (((_activeMenuItem._left * 8) <= cursorPos.x) && (cursorPos.x <= (_activeMenuItem._left * 8 + 80))) { // 80: the width of one menu item on the bar in pixels.
							// If we clicked on the same menu item (the one that is already active) on the bar...
							_vm->_lucerna->_holdLeftMouse = false;
							backup.free();
							return;
						} else {
							_vm->_lucerna->_holdLeftMouse = true;
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
						if (!_vm->_lucerna->_holdLeftMouse)
							break;
					}

					uint16 which = (cursorPos.y - 26) / 20;
					_activeMenuItem.select(which);
					if (_activeMenuItem._options[which]._valid) { // If the menu item wasn't active, we do nothing.
						backup.free();
						return;
					}
				}
			}
		}
	}

	backup.free();
}

bool Menu::isActive() {
	return _menuActive;
}

void Menu::init() {
	_menuActive = false;
}
} // End of namespace Avalanche.
