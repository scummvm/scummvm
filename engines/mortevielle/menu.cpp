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
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#include "mortevielle/mortevielle.h"

#include "mortevielle/menu.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"

#include "common/scummsys.h"
#include "common/str.h"
#include "common/textconsole.h"

namespace Mortevielle {

const byte menuConstants[8][4] = {
	{ 7, 37, 23,  8},
	{19, 33, 23,  7},
	{31, 89, 10, 21},
	{43, 25, 11,  5},
	{55, 37,  5,  8},
	{64, 13, 11,  2},
	{62, 42, 13,  9},
	{62, 46, 13, 10}
};

Menu::Menu(MortevielleEngine *vm) {
	_vm = vm;
	_opcodeAttach = _opcodeWait = _opcodeForce = _opcodeSleep = OPCODE_NONE;
	_opcodeListen = _opcodeEnter = _opcodeClose = _opcodeSearch = OPCODE_NONE;
	_opcodeKnock = _opcodeScratch = _opcodeRead = _opcodeEat = OPCODE_NONE;
	_opcodePlace = _opcodeOpen = _opcodeTake = _opcodeLook = OPCODE_NONE;
	_opcodeSmell = _opcodeSound = _opcodeLeave = _opcodeLift = OPCODE_NONE;
	_opcodeTurn = _opcodeSHide = _opcodeSSearch = _opcodeSRead = OPCODE_NONE;
	_opcodeSPut = _opcodeSLook = _msg3 = _msg4 = OPCODE_NONE;

	_menuActive = false;
	_menuSelected = false;
	_multiTitle = false;
	_menuDisplayed = false;
	for (int i = 0; i < 9; i++) {
		_discussMenu[i]._menuId = MENU_NONE;
		_discussMenu[i]._actionId = 0;
		_inventoryMenu[i]._menuId = MENU_NONE;
		_inventoryMenu[i]._actionId = 0;
	}
	for (int i = 0; i < 8; i++) {
		_moveMenu[i]._menuId = MENU_NONE;
		_moveMenu[i]._actionId = 0;
	}
	for (int i = 0; i < 12; i++) {
		_actionMenu[i]._menuId = MENU_NONE;
		_actionMenu[i]._actionId = 0;
	}
}

void Menu::readVerbNums(Common::File &f, int dataSize) {
	// Figure out what language Id is needed
	byte desiredLanguageId;
	switch(_vm->getLanguage()) {
	case Common::EN_ANY:
		desiredLanguageId = MORTDAT_LANG_ENGLISH;
		break;
	case Common::FR_FRA:
		desiredLanguageId = MORTDAT_LANG_FRENCH;
		break;
	case Common::DE_DEU:
		desiredLanguageId = MORTDAT_LANG_GERMAN;
		break;
	default:
		warning("Language not supported, switching to English");
		desiredLanguageId = MORTDAT_LANG_ENGLISH;
		break;
	}
	// Read in the language
	byte languageId = f.readByte();
	--dataSize;

	// If the language isn't correct, then skip the entire block
	if (languageId != desiredLanguageId) {
		f.skip(dataSize);
		return;
	}

	assert(dataSize == 52);
	_opcodeAttach  = f.readUint16LE();
	_opcodeWait    = f.readUint16LE();
	_opcodeForce   = f.readUint16LE();
	_opcodeSleep   = f.readUint16LE();
	_opcodeListen  = f.readUint16LE();
	_opcodeEnter   = f.readUint16LE();
	_opcodeClose   = f.readUint16LE();
	_opcodeSearch  = f.readUint16LE();
	_opcodeKnock   = f.readUint16LE();
	_opcodeScratch = f.readUint16LE();
	_opcodeRead    = f.readUint16LE();
	_opcodeEat     = f.readUint16LE();
	_opcodePlace   = f.readUint16LE();
	_opcodeOpen    = f.readUint16LE();
	_opcodeTake    = f.readUint16LE();
	_opcodeLook    = f.readUint16LE();
	_opcodeSmell   = f.readUint16LE();
	_opcodeSound   = f.readUint16LE();
	_opcodeLeave   = f.readUint16LE();
	_opcodeLift    = f.readUint16LE();
	_opcodeTurn    = f.readUint16LE();
	_opcodeSHide   = f.readUint16LE();
	_opcodeSSearch = f.readUint16LE();
	_opcodeSRead   = f.readUint16LE();
	_opcodeSPut    = f.readUint16LE();
	_opcodeSLook   = f.readUint16LE();

	_actionMenu[0]._menuId    = OPCODE_NONE   >> 8;
	_actionMenu[0]._actionId  = OPCODE_NONE   & 0xFF;

	_actionMenu[1]._menuId    = _opcodeSHide  >> 8;
	_actionMenu[1]._actionId  = _opcodeSHide  & 0xFF;

	_actionMenu[2]._menuId    = _opcodeAttach >> 8;
	_actionMenu[2]._actionId  = _opcodeAttach & 0xFF;

	_actionMenu[3]._menuId    = _opcodeForce  >> 8;
	_actionMenu[3]._actionId  = _opcodeForce  & 0xFF;

	_actionMenu[4]._menuId    = _opcodeSleep  >> 8;
	_actionMenu[4]._actionId  = _opcodeSleep  & 0xFF;

	_actionMenu[5]._menuId    = _opcodeEnter  >> 8;
	_actionMenu[5]._actionId  = _opcodeEnter  & 0xFF;

	_actionMenu[6]._menuId    = _opcodeClose  >> 8;
	_actionMenu[6]._actionId  = _opcodeClose  & 0xFF;

	_actionMenu[7]._menuId    = _opcodeKnock  >> 8;
	_actionMenu[7]._actionId  = _opcodeKnock  & 0xFF;

	_actionMenu[8]._menuId    = _opcodeEat    >> 8;
	_actionMenu[8]._actionId  = _opcodeEat    & 0xFF;

	_actionMenu[9]._menuId    = _opcodePlace  >> 8;
	_actionMenu[9]._actionId  = _opcodePlace  & 0xFF;

	_actionMenu[10]._menuId   = _opcodeOpen   >> 8;
	_actionMenu[10]._actionId = _opcodeOpen   & 0xFF;

	_actionMenu[11]._menuId   = _opcodeLeave  >> 8;
	_actionMenu[11]._actionId = _opcodeLeave  & 0xFF;
}

/**
 * Setup a menu's contents
 * @remarks	Originally called 'menut'
 */
void Menu::setText(MenuItem item, Common::String name) {
	Common::String s = name;

	switch (item._menuId) {
	case MENU_INVENTORY:
		if (item._actionId != 7) {
			while (s.size() < 22)
				s += ' ';

			_inventoryStringArray[item._actionId] = s;
			_inventoryStringArray[item._actionId].insertChar(' ', 0);
		}
		break;
	case MENU_MOVE: {
		// If the first character isn't '*' or ' ' then it's missing a heading space
		char c = s[0];
		if (c != '*' && c != ' ')
			s.insertChar(' ', 0);

		while (s.size() < 22)
			s += ' ';

		_moveStringArray[item._actionId] = s;
		}
		break;
	case MENU_ACTION: {
		// If the first character isn't '*' or ' ' then it's missing a heading space
		char c = s[0];
		if (c != '*' && c != ' ')
			s.insertChar(' ', 0);

		while (s.size() < 10)
			s += ' ';

		_actionStringArray[item._actionId] = s;
		}
		break;
	case MENU_SELF: {
		// If the first character isn't '*' or ' ' then it's missing a heading space
		char c = s[0];
		if (c != '*' && c != ' ')
			s.insertChar(' ', 0);

		while (s.size() < 10)
			s += ' ';

		_selfStringArray[item._actionId] = s;
		}
		break;
	case MENU_DISCUSS:
		_discussStringArray[item._actionId] = s;
		break;
	default:
		break;
	}
}

/**
 * Init destination menu
 * @remarks	Originally called 'tmlieu'
 */
void Menu::setDestinationText(int roomId) {
	Common::String nomp;

	if (roomId == ROOM26)
		roomId = LANDING;

	int destinationId = 0;
	for (; (destinationId < 7) && (_vm->_destinationArray[destinationId][roomId]); ++destinationId) {
		nomp = _vm->getString(_vm->_destinationArray[destinationId][roomId] + kMenuPlaceStringIndex);
		while (nomp.size() < 20)
			nomp += ' ';
		setText(_moveMenu[destinationId + 1], nomp);
	}
	nomp = "*                   ";
	for (int i = 7; i >= destinationId + 1; --i)
		setText(_moveMenu[i], nomp);
}

/**
 * _disable a menu item
 * @param menuId	Menu number
 * @param actionId  Item index
 */
void Menu::disableMenuItem(MenuItem item) {
	switch (item._menuId) {
	case MENU_INVENTORY:
		if (item._actionId > 6) {
			_inventoryStringArray[item._actionId].setChar('<', 0);
			_inventoryStringArray[item._actionId].setChar('>', 21);
		} else
			_inventoryStringArray[item._actionId].setChar('*', 0);
		break;
	case MENU_MOVE:
		_moveStringArray[item._actionId].setChar('*', 0);
		break;
	case MENU_ACTION:
		_actionStringArray[item._actionId].setChar('*', 0);
		break;
	case MENU_SELF:
		_selfStringArray[item._actionId].setChar('*', 0);
		break;
	case MENU_DISCUSS:
		_discussStringArray[item._actionId].setChar('*', 0);
		break;
	default:
		break;
	}
}

/**
 * Enable a menu item
 * @param menuId	Menu number
 * @param actionId  Item index
 * @remarks	Originally called menu_enable
 */
void Menu::enableMenuItem(MenuItem item) {
	switch (item._menuId) {
	case MENU_INVENTORY:
		_inventoryStringArray[item._actionId].setChar(' ', 0);
		_inventoryStringArray[item._actionId].setChar(' ', 21);
		break;
	case MENU_MOVE:
		_moveStringArray[item._actionId].setChar(' ', 0);
		break;
	case MENU_ACTION:
		_actionStringArray[item._actionId].setChar(' ', 0);
		break;
	case MENU_SELF:
		_selfStringArray[item._actionId].setChar(' ', 0);
		break;
	case MENU_DISCUSS:
		_discussStringArray[item._actionId].setChar(' ', 0);
		break;
	default:
		break;
	}
}

void Menu::displayMenu() {
	_vm->_mouse->hideMouse();
	_vm->_screenSurface->fillRect(7, Common::Rect(0, 0, 639, 10));

	int col = 28 * kResolutionScaler;
	for (int charNum = 0; charNum < 6; charNum++) {
	// One character after the other
		int idx = 0;
		for (int y = 1; y < 9; ++y) {
		// One column after the other
			int x = col;
			for (int k = 0; k < 3; ++k) {
			// One line after the other
				uint msk = 0x80;
				for (int pt = 0; pt <= 7; ++pt) {
					if ((_charArr[charNum][idx] & msk) != 0) {
						_vm->_screenSurface->setPixel(Common::Point(x + 1, y + 1), 0);
						_vm->_screenSurface->setPixel(Common::Point(x, y + 1), 0);
						_vm->_screenSurface->setPixel(Common::Point(x, y), 9);
					}
					msk >>= 1;
					++x;
				}
				++idx;
			}
		}
		col += 48 * kResolutionScaler;
	}
	_vm->_mouse->showMouse();
}

/**
 * Show the menu
 */
void Menu::drawMenu() {
	displayMenu();
	_menuActive = true;
	_msg4 = OPCODE_NONE;
	_msg3 = OPCODE_NONE;
	_menuSelected = false;
	_vm->setMouseClick(false);
	_multiTitle = false;
}

/**
 * Menu function - Invert a menu entry
 * @remarks	Originally called 'invers'
 */
void Menu::invert(int indx) {
	if (_msg4 == OPCODE_NONE)
		return;

	int menuIndex = _msg4 & 0xFF;

	_vm->_screenSurface->putxy(menuConstants[_msg3 - 1][0] << 3, (menuIndex + 1) << 3);

	Common::String str;
	switch (_msg3) {
	case MENU_INVENTORY:
		str = _inventoryStringArray[menuIndex];
		break;
	case MENU_MOVE:
		str = _moveStringArray[menuIndex];
		break;
	case MENU_ACTION:
		str = _actionStringArray[menuIndex];
		break;
	case MENU_SELF:
		str = _selfStringArray[menuIndex];
		break;
	case MENU_DISCUSS:
		str = _discussStringArray[menuIndex];
		break;
	case MENU_FILE:
		str = _vm->getEngineString(S_SAVE_LOAD + menuIndex);
		break;
	case MENU_SAVE:
		str = _vm->getEngineString(S_SAVE_LOAD + 1);
		str += ' ';
		str += (char)(48 + menuIndex);
		break;
	case MENU_LOAD:
		if (menuIndex == 1) {
			str = _vm->getEngineString(S_RESTART);
		} else {
			str = _vm->getEngineString(S_SAVE_LOAD + 2);
			str += ' ';
			str += (char)(47 + menuIndex);
		}
		break;
	default:
		break;
	}
	if ((str[0] != '*') && (str[0] != '<'))
		_vm->_screenSurface->drawString(str, indx);
	else
		_msg4 = OPCODE_NONE;
}

void Menu::util(Common::Point pos) {

	int ymx = (menuConstants[_msg3 - 1][3] << 3) + 16;
	int dxcar = menuConstants[_msg3 - 1][2];
	int xmn = (menuConstants[_msg3 - 1][0] << 2) * kResolutionScaler;

	int charWidth = 6;
	int xmx = dxcar * charWidth + xmn + 2;
	if ((pos.x > xmn) && (pos.x < xmx) && (pos.y < ymx) && (pos.y > 15)) {
		int ix = (((uint)pos.y >> 3) - 1) + (_msg3 << 8);
		if (ix != _msg4) {
			invert(1);
			_msg4 = ix;
			invert(0);
		}
	} else if (_msg4 != OPCODE_NONE) {
		invert(1);
		_msg4 = OPCODE_NONE;
	}
}

/**
 * Draw a menu
 */
void Menu::menuDown(int ii) {
	// Make a copy of the current screen surface for later restore
	_vm->_backgroundSurface.copyFrom(*_vm->_screenSurface);

	// Draw the menu
	int minX = menuConstants[ii - 1][0] << 3;
	int lineNum = menuConstants[ii - 1][3];
	_vm->_mouse->hideMouse();
	int deltaX = 6;
	int maxX = minX + (menuConstants[ii - 1][2] * deltaX) + 6;
	if ((ii == 4) && (_vm->getLanguage() == Common::EN_ANY))
		// Extra width needed for Self menu in English version
		maxX = 435;

	_vm->_screenSurface->fillRect(15, Common::Rect(minX, 12, maxX, 10 + (menuConstants[ii - 1][1] << 1)));
	_vm->_screenSurface->fillRect(0, Common::Rect(maxX, 12, maxX + 4, 10 + (menuConstants[ii - 1][1] << 1)));
	_vm->_screenSurface->fillRect(0, Common::Rect(minX, 8 + (menuConstants[ii - 1][1] << 1), maxX + 4, 12 + (menuConstants[ii - 1][1] << 1)));
	_vm->_screenSurface->putxy(minX, 16);
	for (int i = 1; i <= lineNum; i++) {
		switch (ii) {
		case 1:
			if (_inventoryStringArray[i][0] != '*')
				_vm->_screenSurface->drawString(_inventoryStringArray[i], 4);
			break;
		case 2:
			if (_moveStringArray[i][0] != '*')
				_vm->_screenSurface->drawString(_moveStringArray[i], 4);
			break;
		case 3:
			if (_actionStringArray[i][0] != '*')
				_vm->_screenSurface->drawString(_actionStringArray[i], 4);
			break;
		case 4:
			if (_selfStringArray[i][0] != '*')
				_vm->_screenSurface->drawString(_selfStringArray[i], 4);
			break;
		case 5:
			if (_discussStringArray[i][0] != '*')
				_vm->_screenSurface->drawString(_discussStringArray[i], 4);
			break;
		case 6:
			_vm->_screenSurface->drawString(_vm->getEngineString(S_SAVE_LOAD + i), 4);
			break;
		case 7: {
			Common::String s = _vm->getEngineString(S_SAVE_LOAD + 1);
			s += ' ';
			s += (char)(48 + i);
			_vm->_screenSurface->drawString(s, 4);
			}
			break;
		case 8:
			if (i == 1)
				_vm->_screenSurface->drawString(_vm->getEngineString(S_RESTART), 4);
			else {
				Common::String s = _vm->getEngineString(S_SAVE_LOAD + 2);
				s += ' ';
				s += (char)(47 + i);
				_vm->_screenSurface->drawString(s, 4);
			}
			break;
		default:
			break;
		}
		_vm->_screenSurface->putxy(minX, _vm->_screenSurface->_textPos.y + 8);
	}
	_multiTitle = true;
	_vm->_mouse->showMouse();
}

/**
 * Menu is being removed, so restore the previous background area.
 */
void Menu::menuUp(int msgId) {
	if (_multiTitle) {
		/* Restore the background area */
		assert(_vm->_screenSurface->pitch == _vm->_backgroundSurface.pitch);

		// Get a pointer to the source and destination of the area to restore
		const byte *pSrc = (const byte *)_vm->_backgroundSurface.getBasePtr(0, 10);
		Graphics::Surface destArea = _vm->_screenSurface->lockArea(Common::Rect(0, 10, SCREEN_WIDTH, SCREEN_HEIGHT));
		byte *pDest = (byte *)destArea.getPixels();

		// Copy the data
		Common::copy(pSrc, pSrc + (400 - 10) * SCREEN_WIDTH, pDest);

		_multiTitle = false;
	}
}

/**
 * Erase the menu
 */
void Menu::eraseMenu() {
	_menuActive = false;
	_vm->setMouseClick(false);
	menuUp(_msg3);
}

/**
 * Handle updates to the menu
 * @remarks	Originally called 'mdn'
 */
void Menu::updateMenu() {
	if (!_menuActive)
		return;

	Common::Point curPos = _vm->_mouse->_pos;
	if (!_vm->getMouseClick()) {
		if (curPos == _vm->_prevPos)
			return;
		else
			_vm->_prevPos = curPos;

		bool tes =  (curPos.y < 11)
		   && ((curPos.x >= (28 * kResolutionScaler) && curPos.x <= (28 * kResolutionScaler + 24))
		   ||  (curPos.x >= (76 * kResolutionScaler) && curPos.x <= (76 * kResolutionScaler + 24))
		   || ((curPos.x > 124 * kResolutionScaler) && (curPos.x < 124 * kResolutionScaler + 24))
		   || ((curPos.x > 172 * kResolutionScaler) && (curPos.x < 172 * kResolutionScaler + 24))
		   || ((curPos.x > 220 * kResolutionScaler) && (curPos.x < 220 * kResolutionScaler + 24))
		   || ((curPos.x > 268 * kResolutionScaler) && (curPos.x < 268 * kResolutionScaler + 24)));
		if (tes) {
			int ix;

			if (curPos.x < 76 * kResolutionScaler)
				ix = MENU_INVENTORY;
			else if (curPos.x < 124 * kResolutionScaler)
				ix = MENU_MOVE;
			else if (curPos.x < 172 * kResolutionScaler)
				ix = MENU_ACTION;
			else if (curPos.x < 220 * kResolutionScaler)
				ix = MENU_SELF;
			else if (curPos.x < 268 * kResolutionScaler)
				ix = MENU_DISCUSS;
			else
				ix = MENU_FILE;

			if ((ix != _msg3) || (!_multiTitle))
				if (!((ix == MENU_FILE) && ((_msg3 == MENU_SAVE) || (_msg3 == MENU_LOAD)))) {
					menuUp(_msg3);
					menuDown(ix);
					_msg3 = ix;
					_msg4 = OPCODE_NONE;
				}
		} else { // Not in the MenuTitle line
			if ((curPos.y > 11) && (_multiTitle))
				util(curPos);
		}
	} else {       // There was a click
		if ((_msg3 == MENU_FILE) && (_msg4 != OPCODE_NONE)) {
			// Another menu to be _displayed
			_vm->setMouseClick(false);
			menuUp(_msg3);
			if ((_msg4 & 0xFF) == 1)
				_msg3 = MENU_SAVE;
			else
				_msg3 = MENU_LOAD;
			menuDown(_msg3);

			_vm->setMouseClick(false);
		} else {
			//  A menu was clicked on
			_menuSelected = (_multiTitle) && (_msg4 != OPCODE_NONE);
			menuUp(_msg3);
			_vm->_currAction = _msg4;
			_vm->_currMenu = _msg3;
			_msg3 = OPCODE_NONE;
			_msg4 = OPCODE_NONE;

			_vm->setMouseClick(false);
		}
	}
}

void Menu::initMenu() {
	Common::File f;

	bool menuLoaded = false;
	// First try to read it from mort.dat if useOriginalData() is false
	if (!_vm->useOriginalData()) {
		if (!f.open(MORT_DAT))
			warning("File %s not found. Using default menu from game data", MORT_DAT);
		else {
			// Figure out what language Id is needed
			byte desiredLanguageId;
			switch(_vm->getLanguage()) {
			case Common::EN_ANY:
				desiredLanguageId = MORTDAT_LANG_ENGLISH;
				break;
			case Common::FR_FRA:
				desiredLanguageId = MORTDAT_LANG_FRENCH;
				break;
			case Common::DE_DEU:
				desiredLanguageId = MORTDAT_LANG_GERMAN;
				break;
			default:
				warning("Language not supported, switching to English");
				desiredLanguageId = MORTDAT_LANG_ENGLISH;
				break;
			}

			// Validate the data file header
			char fileId[4];
			f.read(fileId, 4);
			// Do not display warnings here. They would already have been displayed in MortevielleEngine::loadMortDat().
			if (strncmp(fileId, "MORT", 4) == 0 && f.readByte() >= MORT_DAT_REQUIRED_VERSION) {
				f.readByte();		// Minor version
				// Loop to load resources from the data file
				while (f.pos() < f.size()) {
					// Get the Id and size of the next resource
					char dataType[4];
					int dataSize;
					f.read(dataType, 4);
					dataSize = f.readUint16LE();
					if (!strncmp(dataType, "MENU", 4)) {
						// Read in the language
						byte languageId = f.readByte();
						--dataSize;

						// If the language isn't correct, then skip the entire block
						if (languageId != desiredLanguageId) {
							f.skip(dataSize);
							continue;
						}
						if (dataSize == 6 * 24) {
							f.read(_charArr, dataSize);
							menuLoaded = true;
						} else
							warning("Wrong size %d for menu data. Expected %d or less", dataSize, 6 * 24);
						break;
					} else {
						// Other sections
						f.skip(dataSize);
					}
				}
			}
			// Close the file
			f.close();
			if (!menuLoaded)
				warning("Failed to load menu from mort.dat. Will use default menu from game data instead.");
		}
	}

	if (!menuLoaded) {
		// Load menu from game data using the original language
		if (_vm->getOriginalLanguage() == Common::FR_FRA) {
			// We do not support reading menu.mor (it has a different format).
			// If the game version does not have menufr.mor it has to get the menu from the DAT file.
			if (!f.open("menufr.mor"))
				error("Missing file - menufr.mor");
		} else { // Common::DE_DEU
			if (!f.open("menual.mor"))
				error("Missing file - menual.mor");
		}
		f.read(_charArr, 6 * 24);
		f.close();
	}

	// Skipped: dialog asking to swap floppy

	for (int i = 1; i <= 8; ++i)
		_inventoryStringArray[i] = "*                     ";
	_inventoryStringArray[7] = "< -*-*-*-*-*-*-*-*-*- ";
	for (int i = 1; i <= 7; ++i)
		_moveStringArray[i] = "*                       ";
	for (int i = 1; i < 22; i++) {
		_actionStringArray[i] = _vm->getString(i + kMenuActionStringIndex);
		if ((_actionStringArray[i][0] != '*') && (_actionStringArray[i][0] != ' '))
			_actionStringArray[i].insertChar(' ', 0);
		while (_actionStringArray[i].size() < 10)
			_actionStringArray[i] += ' ';

		if (i < 9) {
			if (i < 6) {
				_selfStringArray[i] = _vm->getString(i + kMenuSelfStringIndex);
				if ((_selfStringArray[i][0] != '*') && (_selfStringArray[i][0] != ' '))
					_selfStringArray[i].insertChar(' ', 0);
				while (_selfStringArray[i].size() < 10)
					_selfStringArray[i] += ' ';
			}
			_discussStringArray[i] = _vm->getString(i + kMenuSayStringIndex) + ' ';
		}
	}
	for (int i = 1; i <= 8; ++i) {
		_discussMenu[i]._menuId = MENU_DISCUSS;
		_discussMenu[i]._actionId = i;
		if (i < 8) {
			_moveMenu[i]._menuId = MENU_MOVE;
			_moveMenu[i]._actionId = i;
		}
		_inventoryMenu[i]._menuId = MENU_INVENTORY;
		_inventoryMenu[i]._actionId = i;
		if (i > 6)
			disableMenuItem(_inventoryMenu[i]);
	}
	_msg3 = OPCODE_NONE;
	_msg4 = OPCODE_NONE;
	_vm->_currMenu = OPCODE_NONE;
	_vm->_currAction = OPCODE_NONE;
	_vm->setMouseClick(false);
}

/**
 * Engine function - Switch action menu to "Search" mode
 * @remarks	Originally called 'mfoudi'
 */
void Menu::setSearchMenu() {
	for (int i = 1; i <= 7; ++i)
		disableMenuItem(_moveMenu[i]);

	for (int i = 1; i <= 11; ++i)
		disableMenuItem(_actionMenu[i]);

	MenuItem miSound;
	miSound._menuId   = _opcodeSound >> 8;
	miSound._actionId = _opcodeSound & 0xFF;

	MenuItem miLift;
	miLift._menuId   = _opcodeLift >> 8;
	miLift._actionId = _opcodeLift & 0xFF;

	setText(miSound, _vm->getEngineString(S_SUITE));
	setText(miLift,  _vm->getEngineString(S_STOP));
}

/**
 * Engine function - Switch action menu from "Search" mode back to normal mode
 * @remarks	Originally called 'mfouen'
 */
void Menu::unsetSearchMenu() {
	setDestinationText(_vm->_coreVar._currPlace);
	for (int i = 1; i <= 11; ++i)
		enableMenuItem(_actionMenu[i]);

	MenuItem miSound;
	miSound._menuId   = _opcodeSound >> 8;
	miSound._actionId = _opcodeSound & 0xFF;

	MenuItem miLift;
	miLift._menuId   = _opcodeLift >> 8;
	miLift._actionId = _opcodeLift & 0xFF;

	setText(miSound, _vm->getEngineString(S_PROBE));
	setText(miLift,  _vm->getEngineString(S_RAISE));
}

/**
 * Set Inventory menu texts
 * @remarks	Originally called 'modinv'
 */
void Menu::setInventoryText() {
	Common::String nomp;

	int cy = 0;
	for (int i = 1; i <= 6; ++i) {
		if (_vm->_coreVar._inventory[i] != 0) {
			++cy;
			int r = _vm->_coreVar._inventory[i] + 400;
			nomp = _vm->getString(r - 501 + kInventoryStringIndex);
			setText(_inventoryMenu[cy], nomp);
			enableMenuItem(_inventoryMenu[i]);
		}
	}

	if (cy < 6) {
		for (int i = cy + 1; i <= 6; ++i) {
			setText(_inventoryMenu[i], "                       ");
			disableMenuItem(_inventoryMenu[i]);
		}
	}
}
} // End of namespace Mortevielle
