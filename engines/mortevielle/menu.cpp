/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file _distributed with this source _distribution.
 *
 * This program is free software; you can re_distribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is _distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#include "common/scummsys.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "mortevielle/menu.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"

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

/**
 * Setup a menu's contents
 * @remarks	Originally called 'menut'
 */
void Menu::setText(int menuId, Common::String name) {
	byte h = hi(menuId);
	byte l = lo(menuId);
	Common::String s = name;

	while (s.size() < 22)
		s += ' ';

	switch (h) {
	case MENU_INVENTORY:
		if (l != 7) {
			_inventoryStringArray[l] = s;
			_inventoryStringArray[l].insertChar(' ', 0);
		}
		break;
	case MENU_MOVE:
		_moveStringArray[l] = s;
		break;
	case MENU_ACTION:
		_actionStringArray[l] = s;
		break;
	case MENU_SELF:
		_selfStringArray[l] = s;
		break;
	case MENU_DISCUSS:
		_discussStringArray[l] = s;
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

	if (roomId == 26)
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
 * @param menuId	Hi byte represents menu number, lo byte reprsents item index
 */
void Menu::disableMenuItem(int menuId) {
	byte h = hi(menuId);
	byte l = lo(menuId);

	switch (h) {
	case MENU_INVENTORY:
		if (l > 6) {
			_inventoryStringArray[l].setChar('<', 0);
			_inventoryStringArray[l].setChar('>', 21);
		} else
			_inventoryStringArray[l].setChar('*', 0);
		break;
	case MENU_MOVE:
		_moveStringArray[l].setChar('*', 0);
		break;
	case MENU_ACTION:
		_actionStringArray[l].setChar('*', 0);
		break;
	case MENU_SELF:
		_selfStringArray[l].setChar('*', 0);
		break;
	case MENU_DISCUSS:
		_discussStringArray[l].setChar('*', 0);
		break;
	default:
		break;
	}
}

/**
 * Enable a menu item
 * @param menuId	Hi byte represents menu number, lo byte reprsents item index
 * @remarks	Originally called menu_enable
 */
void Menu::enableMenuItem(int menuId) {
	byte h = hi(menuId);
	byte l = lo(menuId);

	switch (h) {
	case MENU_INVENTORY:
		_inventoryStringArray[l].setChar(' ', 0);
		_inventoryStringArray[l].setChar(' ', 21);
		break;
	case MENU_MOVE:
		_moveStringArray[l].setChar(' ', 0);
		break;
	case MENU_ACTION:
		_actionStringArray[l].setChar(' ', 0);
		break;
	case MENU_SELF:
		_selfStringArray[l].setChar(' ', 0);
		// The original sets two times the same value. Skipped
		// _selfStringArray[l].setChar(' ', 0);
		break;
	case MENU_DISCUSS:
		_discussStringArray[l].setChar(' ', 0);
		break;
	default:
		break;
	}
}

void Menu::displayMenu() {
	int ind_tabl, k, col;

	int pt, x, y, color, msk, num_letr;

	_vm->_mouse.hideMouse();

	_vm->_screenSurface.fillRect(7, Common::Rect(0, 0, 639, 10));
	col = 28 * _vm->_resolutionScaler;
	if (_vm->_currGraphicalDevice == MODE_CGA)
		color = 1;
	else
		color = 9;
	num_letr = 0;
	do {       // One character after the other
		++num_letr;
		ind_tabl = 0;
		y = 1;
		do {     // One column after the other
			k = 0;
			x = col;
			do {   // One line after the other
				msk = 0x80;
				for (pt = 0; pt <= 7; ++pt) {
					if ((_charArr[num_letr - 1][ind_tabl] & msk) != 0) {
						_vm->_screenSurface.setPixel(Common::Point(x + 1, y + 1), 0);
						_vm->_screenSurface.setPixel(Common::Point(x, y + 1), 0);
						_vm->_screenSurface.setPixel(Common::Point(x, y), color);
					}
					msk = (uint)msk >> 1;
					++x;
				}
				++ind_tabl;
				++k;
			} while (k != 3);
			++y;
		} while (y != 9);
		col += 48 * _vm->_resolutionScaler;
	} while (num_letr != 6);
	_vm->_mouse.showMouse();
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

	int menuIndex = lo(_msg4);

	_vm->_screenSurface.putxy(menuConstants[_msg3 - 1][0] << 3, (menuIndex + 1) << 3);

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
		_vm->_screenSurface.drawString(str, indx);
	else
		_msg4 = OPCODE_NONE;
}

void Menu::util(Common::Point pos) {

	int ymx = (menuConstants[_msg3 - 1][3] << 3) + 16;
	int dxcar = menuConstants[_msg3 - 1][2];
	int xmn = (menuConstants[_msg3 - 1][0] << 2) * _vm->_resolutionScaler;

	int ix;
	if (_vm->_resolutionScaler == 1)
		ix = 5;
	else
		ix = 3;
	int xmx = dxcar * ix * _vm->_resolutionScaler + xmn + 2;
	if ((pos.x > xmn) && (pos.x < xmx) && (pos.y < ymx) && (pos.y > 15)) {
		ix = (((uint)pos.y >> 3) - 1) + (_msg3 << 8);
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
	int cx, xcc, xco;
	int lignNumb;

	// Make a copy of the current screen surface for later restore
	_vm->_backgroundSurface.copyFrom(_vm->_screenSurface);

	// Draw the menu
	xco = menuConstants[ii - 1][0];
	lignNumb = menuConstants[ii - 1][3];
	_vm->_mouse.hideMouse();
	xco = xco << 3;
	if (_vm->_resolutionScaler == 1)
		cx = 10;
	else
		cx = 6;
	xcc = xco + (menuConstants[ii - 1][2] * cx) + 6;
	if ((ii == 4) && (_vm->getLanguage() == Common::EN_ANY))
		// Extra width needed for Self menu in English version
		xcc = 435;

	_vm->_screenSurface.fillRect(15, Common::Rect(xco, 12, xcc, 10 + (menuConstants[ii - 1][1] << 1)));
	_vm->_screenSurface.fillRect(0, Common::Rect(xcc, 12, xcc + 4, 10 + (menuConstants[ii - 1][1] << 1)));
	_vm->_screenSurface.fillRect(0, Common::Rect(xco, 8 + (menuConstants[ii - 1][1] << 1), xcc + 4, 12 + (menuConstants[ii - 1][1] << 1)));
	_vm->_screenSurface.putxy(xco, 16);
	cx = 0;
	do {
		++cx;
		switch (ii) {
		case 1:
			if (_inventoryStringArray[cx][0] != '*')
				_vm->_screenSurface.drawString(_inventoryStringArray[cx], 4);
			break;
		case 2:
			if (_moveStringArray[cx][0] != '*')
				_vm->_screenSurface.drawString(_moveStringArray[cx], 4);
			break;
		case 3:
			if (_actionStringArray[cx][0] != '*')
				_vm->_screenSurface.drawString(_actionStringArray[cx], 4);
			break;
		case 4:
			if (_selfStringArray[cx][0] != '*')
				_vm->_screenSurface.drawString(_selfStringArray[cx], 4);
			break;
		case 5:
			if (_discussStringArray[cx][0] != '*')
				_vm->_screenSurface.drawString(_discussStringArray[cx], 4);
			break;
		case 6:
			_vm->_screenSurface.drawString(_vm->getEngineString(S_SAVE_LOAD + cx), 4);
			break;
		case 7: {
			Common::String s = _vm->getEngineString(S_SAVE_LOAD + 1);
			s += ' ';
			s += (char)(48 + cx);
			_vm->_screenSurface.drawString(s, 4);
			}
			break;
		case 8:
			if (cx == 1)
				_vm->_screenSurface.drawString(_vm->getEngineString(S_RESTART), 4);
			else {
				Common::String s = _vm->getEngineString(S_SAVE_LOAD + 2);
				s += ' ';
				s += (char)(47 + cx);
				_vm->_screenSurface.drawString(s, 4);
			}
			break;
		default:
			break;
		}
		_vm->_screenSurface.putxy(xco, _vm->_screenSurface._textPos.y + 8);
	} while (cx != lignNumb);
	_multiTitle = true;
	_vm->_mouse.showMouse();
}

/**
 * Menu is being removed, so restore the previous background area.
 */
void Menu::menuUp(int msgId) {
	if (_multiTitle) {
		/* Restore the background area */
		assert(_vm->_screenSurface.pitch == _vm->_backgroundSurface.pitch);

		// Get a pointer to the source and destination of the area to restore
		const byte *pSrc = (const byte *)_vm->_backgroundSurface.getBasePtr(0, 10);
		Graphics::Surface destArea = _vm->_screenSurface.lockArea(Common::Rect(0, 10, SCREEN_WIDTH, SCREEN_HEIGHT));
		byte *pDest = (byte *)destArea.getBasePtr(0, 0);

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

	Common::Point curPos = _vm->_mouse._pos;
	if (!_vm->getMouseClick()) {
		if (curPos == _vm->_prevPos)
			return;
		else
			_vm->_prevPos = curPos;

		bool tes =  (curPos.y < 11)
		   && ((curPos.x >= (28 * _vm->_resolutionScaler) && curPos.x <= (28 * _vm->_resolutionScaler + 24))
		   ||  (curPos.x >= (76 * _vm->_resolutionScaler) && curPos.x <= (76 * _vm->_resolutionScaler + 24))
		   || ((curPos.x > 124 * _vm->_resolutionScaler) && (curPos.x < 124 * _vm->_resolutionScaler + 24))
		   || ((curPos.x > 172 * _vm->_resolutionScaler) && (curPos.x < 172 * _vm->_resolutionScaler + 24))
		   || ((curPos.x > 220 * _vm->_resolutionScaler) && (curPos.x < 220 * _vm->_resolutionScaler + 24))
		   || ((curPos.x > 268 * _vm->_resolutionScaler) && (curPos.x < 268 * _vm->_resolutionScaler + 24)));
		if (tes) {
			int ix;

			if (curPos.x < 76 * _vm->_resolutionScaler)
				ix = MENU_INVENTORY;
			else if (curPos.x < 124 * _vm->_resolutionScaler)
				ix = MENU_MOVE;
			else if (curPos.x < 172 * _vm->_resolutionScaler)
				ix = MENU_ACTION;
			else if (curPos.x < 220 * _vm->_resolutionScaler)
				ix = MENU_SELF;
			else if (curPos.x < 268 * _vm->_resolutionScaler)
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
			if (lo(_msg4) == 1)
				_msg3 = MENU_SAVE;
			else
				_msg3 = MENU_LOAD;
			menuDown(_msg3);

			_vm->setMouseClick(false);
		} else {
			//  A menu was clicked on
			_menuSelected = (_multiTitle) && (_msg4 != OPCODE_NONE);
			menuUp(_msg3);
			_vm->_msg[4] = _msg4;
			_vm->_msg[3] = _msg3;
			_msg3 = OPCODE_NONE;
			_msg4 = OPCODE_NONE;

			_vm->setMouseClick(false);
		}
	}
}

void Menu::initMenu(MortevielleEngine *vm) {
	_vm = vm;

	int i;
	Common::File f;

	if (!f.open("menufr.mor"))
		if (!f.open("menual.mor"))
			if (!f.open("menu.mor"))
				error("Missing file - menufr.mor or menual.mor or menu.mor");

	f.read(_charArr, 7 * 24);
	f.close();

	// Skipped: dialog asking to swap floppy

	for (i = 1; i <= 8; ++i)
		_inventoryStringArray[i] = "*                     ";
	_inventoryStringArray[7] = "< -*-*-*-*-*-*-*-*-*- ";
	for (i = 1; i <= 7; ++i)
		_moveStringArray[i] = "*                       ";
	i = 1;
	do {
		_actionStringArray[i] = _vm->getString(i + kMenuActionStringIndex);

		while (_actionStringArray[i].size() < 10)
			_actionStringArray[i] += ' ';

		if (i < 9) {
			if (i < 6) {
				_selfStringArray[i] = _vm->getString(i + kMenuSelfStringIndex);
				while (_selfStringArray[i].size() < 10)
					_selfStringArray[i] += ' ';
			}
			_discussStringArray[i] = _vm->getString(i + kMenuSayStringIndex) + ' ';
		}
		++i;
	} while (i != 22);
	for (i = 1; i <= 8; ++i) {
		_discussMenu[i] = 0x500 + i;
		if (i < 8)
			_moveMenu[i] = 0x200 + i;
		_inventoryMenu[i] = 0x100 + i;
		if (i > 6)
			disableMenuItem(_inventoryMenu[i]);
	}
	_msg3 = OPCODE_NONE;
	_msg4 = OPCODE_NONE;
	_vm->_msg[3] = OPCODE_NONE;
	_vm->_msg[4] = OPCODE_NONE;
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

	setText(OPCODE_SOUND, _vm->getEngineString(S_SUITE));
	setText(OPCODE_LIFT, _vm->getEngineString(S_STOP));
}

/**
 * Engine function - Switch action menu from "Search" mode back to normal mode
 * @remarks	Originally called 'mfouen'
 */
void Menu::unsetSearchMenu() {
	setDestinationText(_vm->_coreVar._currPlace);
	for (int i = 1; i <= 11; ++i)
		enableMenuItem(_actionMenu[i]);

	setText(OPCODE_SOUND, _vm->getEngineString(S_PROBE));
	setText(OPCODE_LIFT, _vm->getEngineString(S_RAISE));
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
