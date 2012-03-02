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
 * Copyright (c) 1988-1989 Lankhor
 */

#include "common/scummsys.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "mortevielle/level15.h"
#include "mortevielle/menu.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/ovd1.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

/* NIVEAU 14*/

/**
 * Setup a menu's contents
 */
void Menu::menut(int no, Common::String name) {
	byte h = hi(no);
	byte l = lo(no);
	Common::String s = name;

	if (!g_tesok)
		g_vm->quitGame();


	while (s.size() < 20)
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
 * _disable a menu item
 * @param no	Hi byte represents menu number, lo byte reprsents item index
 */
void Menu::disableMenuItem(int no) {
	byte h = hi(no);
	byte l = lo(no);

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
 * @param no	Hi byte represents menu number, lo byte reprsents item index
 * @remarks	Originally called menu_enable
 */
void Menu::enableMenuItem(int no) {
	byte h = hi(no);
	byte l = lo(no);

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

	hideMouse();
	
	g_vm->_screenSurface.fillRect(7, Common::Rect(0, 0, 639, 10));
	col = 28 * g_res;
	if (g_currGraphicalDevice == MODE_CGA)
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
					if ((lettres[num_letr - 1][ind_tabl] & msk) != 0) {
						g_vm->_screenSurface.setPixel(Common::Point(x + 1, y + 1), 0);
						g_vm->_screenSurface.setPixel(Common::Point(x, y + 1), 0);
						g_vm->_screenSurface.setPixel(Common::Point(x, y), color);
					}
					msk = (uint)msk >> 1;
					++x;
				}
				++ind_tabl;
				++k;
			} while (k != 3);
			++y;
		} while (y != 9);
		col += 48 * g_res;
	} while (num_letr != 6);
	showMouse();
}

/**
 * Show the menu
 */
void Menu::drawMenu() {
	displayMenu();
	_menuActive = true;
	g_msg4 = OPCODE_NONE;
	g_msg3 = OPCODE_NONE;
	g_choisi = false;
	g_vm->setMouseClick(false);
	g_test0 = false;
}

void Menu::invers(int ix) {
	Common::String s;

	if (g_msg4 == OPCODE_NONE)
		return;

	int menuIndex = lo(g_msg4);

	g_vm->_screenSurface.putxy(_menuConstants[g_msg3 - 1][0] << 3, (menuIndex + 1) << 3);
	switch (g_msg3) {
	case 1:
		s = _inventoryStringArray[menuIndex];
		break;
	case 2:
		s = _moveStringArray[menuIndex];
		break;
	case 3:
		s = _actionStringArray[menuIndex];
		break;
	case 4:
		s = _selfStringArray[menuIndex];
		break;
	case 5:
		s = _discussStringArray[menuIndex];
		break;
	case 6:
		s = g_vm->getEngineString(S_SAVE_LOAD + menuIndex);
		break;
	case 7:
		s = g_vm->getEngineString(S_SAVE_LOAD + 1);
		s += ' ';
		s += (char)(48 + menuIndex);
		break;
	case 8:
		if (menuIndex == 1) {
			s = g_vm->getEngineString(S_RESTART);
		} else {
			s = g_vm->getEngineString(S_SAVE_LOAD + 2);
			s += ' ';
			s += (char)(47 + menuIndex);
		}
		break;
	default:
		break;
	}
	if ((s[0] != '*') && (s[0] != '<'))
		g_vm->_screenSurface.writeg(s, ix);
	else
		g_msg4 = OPCODE_NONE;
}

void Menu::util(int x, int y) {

	int ymx = (_menuConstants[g_msg3 - 1][3] << 3) + 16;
	int dxcar = _menuConstants[g_msg3 - 1][2];
	int xmn = (_menuConstants[g_msg3 - 1][0] << 2) * g_res;

	int ix;
	if (g_res == 1)
		ix = 5;
	else
		ix = 3;
	int xmx = dxcar * ix * g_res + xmn + 2;
	if ((x > xmn) && (x < xmx) && (y < ymx) && (y > 15)) {
		ix = (((uint)y >> 3) - 1) + (g_msg3 << 8);
		if (ix != g_msg4) {
			invers(1);
			g_msg4 = ix;
			invers(0);
		}
	} else if (g_msg4 != OPCODE_NONE) {
		invers(1);
		g_msg4 = OPCODE_NONE;
	}
}

/**
 * Draw a menu
 */
void Menu::menuDown(int ii) {
	int cx, xcc;
	int xco, nb_lig;

	/* debug('menuDown'); */

	// Make a copy of the current screen surface for later restore
	g_vm->_backgroundSurface.copyFrom(g_vm->_screenSurface);

	// Draw the menu
	xco = _menuConstants[ii - 1][0];
	nb_lig = _menuConstants[ii - 1][3];
	hideMouse();
	sauvecr(10, (_menuConstants[ii - 1][1] + 1) << 1);
	xco = xco << 3;
	if (g_res == 1)
		cx = 10;
	else
		cx = 6;
	xcc = xco + (_menuConstants[ii - 1][2] * cx) + 6;
	g_vm->_screenSurface.fillRect(15, Common::Rect(xco, 12, xcc, 10 + (_menuConstants[ii - 1][1] << 1)));
	g_vm->_screenSurface.fillRect(0, Common::Rect(xcc, 12, xcc + 4, 10 + (_menuConstants[ii - 1][1] << 1)));
	g_vm->_screenSurface.fillRect(0, Common::Rect(xco, 8 + (_menuConstants[ii - 1][1] << 1), xcc + 4, 12 + (_menuConstants[ii - 1][1] << 1)));
	g_vm->_screenSurface.putxy(xco, 16);
	cx = 0;
	do {
		++cx;
		switch (ii) {
		case 1:
			if (_inventoryStringArray[cx][0] != '*')
				g_vm->_screenSurface.writeg(_inventoryStringArray[cx], 4);
			break;
		case 2:
			if (_moveStringArray[cx][0] != '*')
				g_vm->_screenSurface.writeg(_moveStringArray[cx], 4);
			break;
		case 3:
			if (_actionStringArray[cx][0] != '*')
				g_vm->_screenSurface.writeg(_actionStringArray[cx], 4);
			break;
		case 4:
			if (_selfStringArray[cx][0] != '*')
				g_vm->_screenSurface.writeg(_selfStringArray[cx], 4);
			break;
		case 5:
			if (_discussStringArray[cx][0] != '*')
				g_vm->_screenSurface.writeg(_discussStringArray[cx], 4);
			break;
		case 6:
			g_vm->_screenSurface.writeg(g_vm->getEngineString(S_SAVE_LOAD + cx), 4);
			break;
		case 7: {
			Common::String s = g_vm->getEngineString(S_SAVE_LOAD + 1);
			s += ' ';
			s += (char)(48 + cx);
			g_vm->_screenSurface.writeg(s, 4);
			}
			break;
		case 8:
			if (cx == 1)
				g_vm->_screenSurface.writeg(g_vm->getEngineString(S_RESTART), 4);
			else {
				Common::String s = g_vm->getEngineString(S_SAVE_LOAD + 2);
				s += ' ';
				s += (char)(47 + cx);
				g_vm->_screenSurface.writeg(s, 4);
			}
			break;
		default:
			break;
		}
		g_vm->_screenSurface.putxy(xco, g_vm->_screenSurface._textPos.y + 8);
	} while (cx != nb_lig);
	g_test0 = true;
	showMouse();
}

/**
 * Menu is being removed, so restore the previous background area.
 */
void Menu::menuUp(int xx) {
	/* debug('menuUp'); */
	if (g_test0) {
		charecr(10, (_menuConstants[xx - 1][1] + 1) << 1);

		/* Restore the background area */
		assert(g_vm->_screenSurface.pitch == g_vm->_backgroundSurface.pitch);

		// Get a pointer to the source and destination of the area to restore
		const byte *pSrc = (const byte *)g_vm->_backgroundSurface.getBasePtr(0, 10);
		Graphics::Surface destArea = g_vm->_screenSurface.lockArea(Common::Rect(0, 10, SCREEN_WIDTH, SCREEN_HEIGHT));
		byte *pDest = (byte *)destArea.getBasePtr(0, 0);

		// Copy the data
		Common::copy(pSrc, pSrc + (400 - 10) * SCREEN_WIDTH, pDest);

		g_test0 = false;
	}
}

/**
 * Erase the menu
 */
void Menu::eraseMenu() {
	/* debug('eraseMenu'); */
	_menuActive = false;
	g_vm->setMouseClick(false);
	menuUp(g_msg3);
}

/**
 * Handle updates to the menu
 */
void Menu::mdn() {
	if (!_menuActive)
		return;

	int x = x_s;
	int y = y_s;
	if (!g_vm->getMouseClick()) {
		if ((x == g_xprec) && (y == g_yprec))
			return;
		else {
			g_xprec = x;
			g_yprec = y;
		}
		
		bool tes =  (y < 11) 
		   && ((x >= (28 * g_res) && x <= (28 * g_res + 24)) 
		   ||  (x >= (76 * g_res) && x <= (76 * g_res + 24))
		   || ((x > 124 * g_res) && (x < 124 * g_res + 24))
		   || ((x > 172 * g_res) && (x < 172 * g_res + 24))
		   || ((x > 220 * g_res) && (x < 220 * g_res + 24))
		   || ((x > 268 * g_res) && (x < 268 * g_res + 24)));
		if (tes) {
			int ix;

			if (x < 76 * g_res)
				ix = MENU_INVENTORY;
			else if (x < 124 * g_res)
				ix = MENU_MOVE;
			else if (x < 172 * g_res)
				ix = MENU_ACTION;
			else if (x < 220 * g_res)
				ix = MENU_SELF;
			else if (x < 268 * g_res)
				ix = MENU_DISCUSS;
			else
				ix = MENU_FILE;

			if ((ix != g_msg3) || (!g_test0))
				if (!((ix == MENU_FILE) && ((g_msg3 == MENU_SAVE) || (g_msg3 == MENU_LOAD)))) {
					menuUp(g_msg3);
					menuDown(ix);
					g_msg3 = ix;
					g_msg4 = OPCODE_NONE;
				}
		} else { // Not in the MenuTitle line
			if ((y > 11) && (g_test0))
				util(x, y);
		}
	} else {       // There was a click
		if ((g_msg3 == MENU_FILE) && (g_msg4 != OPCODE_NONE)) {
			// Another menu to be _displayed
			g_vm->setMouseClick(false);
			menuUp(g_msg3);
			if (lo(g_msg4) == 1)
				g_msg3 = 7;
			else
				g_msg3 = 8;
			menuDown(g_msg3);

			g_vm->setMouseClick(false);
		} else { 
			//  A menu was clicked on
			g_choisi = (g_test0) && (g_msg4 != OPCODE_NONE);
			menuUp(g_msg3);
			g_msg[4] = g_msg4;
			g_msg[3] = g_msg3;
			g_msg3 = OPCODE_NONE;
			g_msg4 = OPCODE_NONE;

			g_vm->setMouseClick(false);
		}
	}
}

void Menu::initMenu() {
	int i;
	Common::File f;

	if (!f.open("menufr.mor"))
		if (!f.open("menual.mor"))
			if (!f.open("menu.mor"))
				error("Missing file - menufr.mor or menual.mor or menu.mor");

	f.read(lettres, 7 * 24);
	f.close();

	// Ask to swap floppy
	dem2();

	for (i = 1; i <= 8; ++i)
		_inventoryStringArray[i] = "*                     ";
	_inventoryStringArray[7] = "< -*-*-*-*-*-*-*-*-*- ";
	for (i = 1; i <= 7; ++i)
		_moveStringArray[i] = "*                       ";
	i = 1;
	do {
		_actionStringArray[i] = deline(i + c_action);

		while (_actionStringArray[i].size() < 10)
			_actionStringArray[i] += ' ';

		if (i < 9) {
			if (i < 6) {
				_selfStringArray[i] = deline(i + c_saction);
				while (_selfStringArray[i].size() < 10)
					_selfStringArray[i] += ' ';
			}
			_discussStringArray[i] = deline(i + c_dis) + ' ';
		}
		++i;
	} while (i != 22);
	for (i = 1; i <= 8; ++i) {
		_discussMenu[i] = 0x500 + i;
		if (i < 8)
			_moveMenu[i] = 0x200 + i;
		_inventoryMenu[i] = 0x100 + i;
		if (i > 6)
			g_vm->_menu.disableMenuItem(_inventoryMenu[i]);
	}
	g_msg3 = OPCODE_NONE;
	g_msg4 = OPCODE_NONE;
	g_msg[3] = OPCODE_NONE;
	g_msg[4] = OPCODE_NONE;
	g_vm->setMouseClick(false);
}

} // End of namespace Mortevielle
