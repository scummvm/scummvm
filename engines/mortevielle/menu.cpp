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
void Menu::menut(int no, Common::String nom) {
	byte h, l;
	Common::String s;

	h = hi(no);
	l = lo(no);
	s = nom;
	if (! tesok) {
		g_vm->quitGame();
	}
	while (s.size() < 20)
		s = s + ' ';

	switch (h) {
	case invent  :
		if (l != 7) {
			_inv[l] = s;
			_inv[l].insertChar(' ', 0);
		}
		break;
	case depla:
		_dep[l] = s;
		break;
	case action:
		_act[l] = s;
		break;
	case saction:
		_self[l] = s;
		break;
	case discut:
		_dis[l] = s;
		break;
	}
}

/**
 * _disable a menu item
 * @param no	Hi byte represents menu number, lo byte reprsents item index
 */
void Menu::disableMenuItem(int no) {
	byte h, l;

	h = hi(no);
	l = lo(no);
	switch (h) {
	case invent : {
		if (l > 6)  {
			_inv[l].setChar('<', 0);
			_inv[l].setChar('>', 21);
		} else
			_inv[l].setChar('*', 0);
	}
	break;
	case depla:
		_dep[l].setChar('*', 0);
		break;
	case action:
		_act[l].setChar('*', 0);
		break;
	case saction:
		_self[l].setChar('*', 0);
		break;
	case discut:
		_dis[l].setChar('*', 0);
		break;
	}
}

/**
 * Enable a menu item
 * @param no	Hi byte represents menu number, lo byte reprsents item index
 */
void Menu::enableMenuItem(int no) {
	byte h, l;

	h = hi(no);
	l = lo(no);
	switch (h) {
	case invent : {
		_inv[l].setChar(' ', 0);
		_inv[l].setChar(' ', 21);
	}
	break;
	case depla:
		_dep[l].setChar(' ', 0);
		break;
	case action:
		_act[l].setChar(' ', 0);
		break;
	case saction: {
		_self[l].setChar(' ', 0);
		_self[l].setChar(' ', 0);
	}
	break;
	case discut:
		_dis[l].setChar(' ', 0);
		break;
	}
}

void Menu::menu_aff() {
	int ind_tabl, k, col;

	int pt, x, y, color, msk, num_letr;

	hideMouse();
	
	g_vm->_screenSurface.fillRect(7, Common::Rect(0, 0, 639, 10));
	col = 28 * res;
	if (gd == cga)  color = 1;
	else color = 9;
	num_letr = 0;
	do {       /* lettre par lettre */
		num_letr = num_letr + 1;
		ind_tabl = 0;
		y = 1;
		do {      /* colonne par colonne */
			k = 0;
			x = col;
			do {     /* ligne par ligne */
				msk = 0x80;
				for (pt = 0; pt <= 7; pt ++) {
					if ((lettres[num_letr - 1][ind_tabl] & msk) != 0) {
						g_vm->_screenSurface.setPixel(Common::Point(x + 1, y + 1), 0);
						g_vm->_screenSurface.setPixel(Common::Point(x, y + 1), 0);
						g_vm->_screenSurface.setPixel(Common::Point(x, y), color);
					}
					msk = (uint)msk >> 1;
					x = x + 1;
				}
				ind_tabl = succ(int, ind_tabl);
				k = succ(int, k);
			} while (!(k == 3));
			y = y + 1;
		} while (!(y == 9));
		col = col + 48 * res;
	} while (!(num_letr == 6));
	showMouse();
}

/**
 * Show the menu
 */
void Menu::drawMenu() {
	menu_aff();
	_menuActive = true;
	msg4 = no_choice;
	msg3 = no_choice;
	choisi = false;
	g_vm->setMouseClick(false);
	test0 = false;
}

void Menu::invers(int ix) {
	Common::String s;

	if (msg4 == no_choice)  return;
	g_vm->_screenSurface.putxy(don[msg3][1] << 3, succ(void, lo(msg4)) << 3);
	switch (msg3) {
	case 1 :
		s = _inv[lo(msg4)];
		break;
	case 2 :
		s = _dep[lo(msg4)];
		break;
	case 3 :
		s = _act[lo(msg4)];
		break;
	case 4 :
		s = _self[lo(msg4)];
		break;
	case 5 :
		s = _dis[lo(msg4)];
		break;
	case 6 :
		s = g_vm->getString(S_SAVE_LOAD + lo(msg4));
		break;
	case 7 :
		s = g_vm->getString(S_SAVE_LOAD + 1);
		s += ' ';
		s += (char)(48 + lo(msg4));
		break;
	case 8 :
		if (lo(msg4) == 1) {
			s = g_vm->getString(S_RESTART);
		} else {
			s = g_vm->getString(S_SAVE_LOAD + 2);
			s += ' ';
			s += (char)(47 + lo(msg4));
		}
		break;
	}
	if ((s[0] != '*') && (s[0] != '<'))
		g_vm->_screenSurface.writeg(s, ix);
	else
		msg4 = no_choice;
}

void Menu::util(int x, int y) {
	int ymx, dxcar, xmn, xmx, ix;

	/* debug('util'); */
	ymx = (don[msg3][4] << 3) + 16;
	dxcar = don[msg3][3];
	xmn = (don[msg3][1] << 2) * res;
	if (res == 1)  ix = 5;
	else ix = 3;
	xmx = dxcar * ix * res + xmn + 2;
	if ((x > xmn) && (x < xmx) && (y < ymx) && (y > 15)) {
		ix = pred(int, ((uint)y >> 3)) + (msg3 << 8);
		if (ix != msg4) {
			invers(1);
			msg4 = ix;
			invers(0);
		}
	} else if (msg4 != no_choice) {
		invers(1);
		msg4 = no_choice;
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
	xco = don[ii][1];
	nb_lig = don[ii][4];
	hideMouse();
	sauvecr(10, succ(byte, don[ii][2]) << 1);
	xco = xco << 3;
	if (res == 1)  cx = 10;
	else cx = 6;
	xcc = xco + (don[ii][3] * cx) + 6;
	g_vm->_screenSurface.fillRect(15, Common::Rect(xco, 12, xcc, 10 + (don[ii][2] << 1)));
	g_vm->_screenSurface.fillRect(0, Common::Rect(xcc, 12, xcc + 4, 10 + (don[ii][2] << 1)));
	g_vm->_screenSurface.fillRect(0, Common::Rect(xco, 8 + (don[ii][2] << 1), xcc + 4, 12 + (don[ii][2] << 1)));
	g_vm->_screenSurface.putxy(xco, 16);
	cx = 0;
	do {
		cx = succ(int, cx);
		switch (ii) {
		case 1 :
			if (_inv[cx][0] != '*')
				g_vm->_screenSurface.writeg(_inv[cx], 4);
			break;
		case 2 :
			if (_dep[cx][0] != '*')
				g_vm->_screenSurface.writeg(_dep[cx], 4);
			break;
		case 3 :
			if (_act[cx][0] != '*')
				g_vm->_screenSurface.writeg(_act[cx], 4);
			break;
		case 4 :
			if (_self[cx][0] != '*')
				g_vm->_screenSurface.writeg(_self[cx], 4);
			break;
		case 5 :
			if (_dis[cx][0] != '*')
				g_vm->_screenSurface.writeg(_dis[cx], 4);
			break;
		case 6 :
			g_vm->_screenSurface.writeg(g_vm->getString(S_SAVE_LOAD + cx), 4);
			break;
		case 7 : {
			Common::String s = g_vm->getString(S_SAVE_LOAD + 1);
			s += ' ';
			s += (char)(48 + cx);
			g_vm->_screenSurface.writeg(s, 4);
			break;
		}
		case 8 :
			if (cx == 1)
				g_vm->_screenSurface.writeg(g_vm->getString(S_RESTART), 4);
			else {
				Common::String s = g_vm->getString(S_SAVE_LOAD + 2);
				s += ' ';
				s += (char)(47 + cx);
				g_vm->_screenSurface.writeg(s, 4);
			}
			break;
		}
		g_vm->_screenSurface.putxy(xco, g_vm->_screenSurface._textPos.y + 8);
	} while (!(cx == nb_lig));
	test0 = true;
	showMouse();
}

/**
 * Menu is being removed, so restore the previous background area.
 */
void Menu::menuUp(int xx) {
	/* debug('menuUp'); */
	if (test0) {
		charecr(10, succ(byte, don[xx][2]) << 1);

		/* Restore the background area */
		assert(g_vm->_screenSurface.pitch == g_vm->_backgroundSurface.pitch);

		// Get a pointer to the source and destination of the area to restore
		const byte *pSrc = (const byte *)g_vm->_backgroundSurface.getBasePtr(0, 10);
		Graphics::Surface destArea = g_vm->_screenSurface.lockArea(Common::Rect(0, 10, SCREEN_WIDTH, SCREEN_HEIGHT));
		byte *pDest = (byte *)destArea.getBasePtr(0, 0);

		// Copy the data
		Common::copy(pSrc, pSrc + (400 - 10) * SCREEN_WIDTH, pDest);

		test0 = false;
	}
}

/**
 * Erase the menu
 */
void Menu::eraseMenu() {
	/* debug('eraseMenu'); */
	_menuActive = false;
	g_vm->setMouseClick(false);
	menuUp(msg3);
}

/**
 * Handle updates to the menu
 */
void Menu::mdn() {
	//int x, y, c, a, ix;
	int x, y, ix;
	bool tes;

	/* debug('mdn'); */
	if (!_menuActive)
		return;
	x = x_s;
	y = y_s;
	if (!g_vm->getMouseClick()) {
		if ((x == xprec) &&
		        (y == yprec))  return;
		else {
			xprec = x;
			yprec = y;
		}
		tes = (y < 11) && ((x >= (28 * res) && x <= (28 * res + 24)) 
						   || (x >= (76 * res) && x <= (76 * res + 24))
		                   || ((x > 124 * res) && (x < 124 * res + 24))
		                   || ((x > 172 * res) && (x < 172 * res + 24))
		                   || ((x > 220 * res) && (x < 220 * res + 24))
		                   || ((x > 268 * res) && (x < 268 * res + 24)));
		if (tes) {
			if (x < 76 * res)  ix = invent;
			else if (x < 124 * res)  ix = depla;
			else if (x < 172 * res)  ix = action;
			else if (x < 220 * res)  ix = saction;
			else if (x < 268 * res)  ix = discut;
			else ix = fichier;
			if ((ix != msg3) || (! test0))
				if (!((ix == fichier) && ((msg3 == sauve) || (msg3 == charge)))) {
					menuUp(msg3);
					menuDown(ix);
					msg3 = ix;
					msg4 = no_choice;
				}
		} else { /* Not in the MenuTitle line */
			if ((y > 11) && (test0))  util(x, y);
		}
	} else {       /* There was a click */
		if ((msg3 == fichier) && (msg4 != no_choice)) {
			// Another menu to be _displayed
			g_vm->setMouseClick(false);
			menuUp(msg3);
			if (lo(msg4) == 1)  msg3 = 7;
			else msg3 = 8;
			menuDown(msg3);

			g_vm->setMouseClick(false);
		} else { 
			//  A menu was clicked on
			choisi = (test0) && (msg4 != no_choice);
			menuUp(msg3);
			msg[4] = msg4;
			msg[3] = msg3;
			msg3 = no_choice;
			msg4 = no_choice;

			g_vm->setMouseClick(false);
		}
	}
}

void Menu::initMenu() {
	int i, tai;
	char st[1410];
	Common::File f;

	if (!f.open("menufr.mor"))
		error("Missing file - menufr.mor");

	f.read(lettres, 7 * 24);
	f.close();

	// Ask to swap floppy
	dem2();

	for (i = 1; i <= 8; i ++)
		_inv[i] = "*                     ";
	_inv[7] = "< -*-*-*-*-*-*-*-*-*- ";
	for (i = 1; i <= 7; i ++)
		_dep[i] = "*                       ";
	i = 1;
	do {
		deline(i + c_action, st, tai);
		_act[i] = delig;
		while (_act[i].size() < 10)
			_act[i] += ' ';

		if (i < 9) {
			if (i < 6) {
				deline(i + c_saction, st, tai);
				_self[i] = delig;
				while (_self[i].size() < 10)
					_self[i] += ' ';
			}
			deline(i + c_dis, st, tai);
			_dis[i] = delig + ' ';
		}
		i = succ(int, i);
	} while (!(i == 22));
	for (i = 1; i <= 8; i ++) {
		_disc[i] = 0x500 + i;
		if (i < 8)
			_depl[i] = 0x200 + i;
		_invt[i] = 0x100 + i;
		if (i > 6)
			g_vm->_menu.disableMenuItem(_invt[i]);
	}
	msg3 = no_choice;
	msg4 = no_choice;
	msg[3] = no_choice;
	msg[4] = no_choice;
	g_vm->setMouseClick(false);
}

} // End of namespace Mortevielle
