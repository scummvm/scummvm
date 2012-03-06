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

#include "mortevielle/mor2.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/keyboard.h"
#include "mortevielle/menu.h"
#include "mortevielle/mor.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/parole2.h"
#include "mortevielle/taffich.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

void tinke() {
	Common::String d1 = g_vm->getEngineString(S_SHOULD_HAVE_NOTICED);
	Common::String d2 = g_vm->getEngineString(S_NUMBER_OF_HINTS);
	const char d3 = '[';
	const char d4 = ']';
	const char d5 = '1';
	Common::String d6 = g_vm->getEngineString(S_OK);
	int cx, cf, day, hour, minute;
	Common::String stpo;

	g_anyone = false;
	updateHour(day, hour, minute);
	if (day != g_day) {
		g_day = day;
		cx = 0;
		do {
			++cx;
			if (g_nbrepm[cx] != 0)
				--g_nbrepm[cx];
			g_nbrep[cx] = 0;
		} while (cx != 8);
	}
	if ((hour > g_hour) || ((hour == 0) && (g_hour == 23))) {
		g_hour = hour;
		g_minute = 0;
		drawClock();
		cf = 0;
		for (cx = 1; cx <= 10; ++cx) {
			if (g_s._pourc[cx] == '*')
				++cf;
		}

		if (cf == 10)
			stpo = "10";
		else
			stpo = chr(cf + 48);

		g_hintPctMessage = Common::String(d3);
		g_hintPctMessage += d5;
		g_hintPctMessage += d4;
		g_hintPctMessage += d3;
		g_hintPctMessage += d1;
		g_hintPctMessage += stpo;
		g_hintPctMessage += '0';
		g_hintPctMessage += d2;
		g_hintPctMessage += d4;
		g_hintPctMessage += d3;
		g_hintPctMessage += d6;
		g_hintPctMessage += d4;
	}
	if (minute > g_minute) {
		g_minute = 30;
		drawClock();
	}
	if (y_s < 12)
		return;

	if (!g_blo) {
		if ((hour == 12) || ((hour > 18) && (hour < 21)) || ((hour >= 0) && (hour < 7)))
			g_t = ti2;
		else
			g_t = ti1;
		cf = g_s._faithScore;
		if ((cf > 33) && (cf < 66))
			g_t -= (g_t / 3);

		if (cf > 65)
			g_t -= ((g_t / 3) * 2);

		int nh = readclock();
		if ((nh - g_mh) > g_t) {
			bool activeMenu = g_vm->_menu._menuActive;
			g_vm->_menu.eraseMenu();
			g_jh += ((nh - g_mh) / g_t);
			g_mh = nh;
			switch (g_li) {
			case 1:
			case 4 :
				pl1(cf);
				break;
			case 2 :
				pl2(cf);
				break;
			case 5 :
				pl5(cf);
				break;
			case 6:
			case 8 :
				pl6(cf);
				break;
			case 9 :
				pl9(cf);
				break;
			case 10 :
				pl10(cf);
				break;
			case 11 :
				pl11(cf);
				break;
			case 12 :
				pl12(cf);
				break;
			case 13:
			case 14 :
				pl13(cf);
				break;
			case 15:
			case 26 :
				pl15(cf);
				break;
			case 20 :
				pl20(cf);
				break;
			}
			if ((g_mpers != 0) && (g_ipers != 10))
				g_mpers = g_ipers;

			if ((g_mpers == 0) && (g_ipers > 0)) {
				if ((g_s._currPlace == ATTIC) || (g_s._currPlace == CELLAR)) {
					cavegre();
				} else if (g_ipers == 10) {
					g_ipers = 0;
					if (!g_brt) {
						g_brt = true;
						g_hdb = readclock();
						if (getRandomNumber(1, 5) < 5) {
							clsf3();
							ecrf2();
							ecr3(g_vm->getEngineString(S_HEAR_NOISE));
							int rand = (getRandomNumber(0, 4)) - 2;
							parole(1, rand, 1);
							clsf3();
						}
					}
				}
			}

			if (activeMenu)
				g_vm->_menu.drawMenu();
		}
	}
	g_hfb = readclock();
	if ((g_brt) && ((g_hfb - g_hdb) > 17)) {
		nouvp(g_li, cx);
		g_brt = false;
		g_hdb = 0;
		if ((g_s._currPlace > OWN_ROOM) && (g_s._currPlace < DINING_ROOM))
			g_anyone = true;
	}
}

void fenat(char ans) {
	int coul;

	hideMouse();
	if (g_currGraphicalDevice == MODE_CGA)
		coul = 2;
	else if (g_currGraphicalDevice == MODE_HERCULES)
		coul = 1;
	else
		coul = 12;

	g_vm->_screenSurface.writeCharacter(Common::Point(306, 193), ord(ans), coul);
	g_vm->_screenSurface.drawBox(300, 191, 16, 8, 15);
	showMouse();
}


/* NIVEAU 8 */
void afdes(int ad) {
	taffich();
	dessin(ad);
	g_okdes = false;
}

void tkey1(bool d) {
	bool quest;
	int x, y, c;

	hideMouse();
	fenat('K');

	// Wait for release from any key or mouse button
	while (keypressed())
		g_key = testou();
	do {
		getMousePos(x, y, c);
		keypressed();
	} while (c != 0);
	
	// Event loop
	do {
		if (d)
			tinke();
		quest = keypressed();
		getMousePos(x, y, c);
		CHECK_QUIT;
	} while (!(quest || (c != 0) || (d && g_anyone)));
	if (quest)
		testou();
	g_vm->setMouseClick(false);
	showMouse();
}

void tmlieu(int roomId) {
	Common::String nomp;

	if (roomId == 26)
		roomId = 15;

	int i = 1;
	while ((i < 8) && (g_v_lieu[i][roomId] != 0)) {
		nomp = deline(g_v_lieu[i][roomId] + kMenuPlaceStringIndex);
		while (nomp.size() < 20)
			nomp += ' ';
		g_vm->_menu.menut(g_vm->_menu._moveMenu[i], nomp);
		++i;
	}
	nomp = "*                   ";
	for (int cx = 7; cx >= i; --cx)
		g_vm->_menu.menut(g_vm->_menu._moveMenu[cx], nomp);
}


/* NIVEAU 7 */
void tlu(int af, int ob) {
	g_caff = 32;
	afdes(0);
	repon(6, ob + 4000);
	repon(2, 999);
	tkey1(true);
	g_caff = af;
	g_msg[3] = OPCODE_NONE;
	g_crep = 998;
}

void affrep() {
	g_caff = g_s._currPlace;
	g_crep = g_s._currPlace;
}

/**
 * Engine function - Switch action menu from "Search" mode back to normal mode
 * @remarks	Originally called 'mfouen'
 */
void unsetSearchMenu() {
	tmlieu(g_s._currPlace);
	for (int cx = 1; cx <= 11; ++cx)
		g_vm->_menu.enableMenuItem(_actionMenu[cx]);

	g_vm->_menu.menut(OPCODE_SOUND, g_vm->getEngineString(S_PROBE));
	g_vm->_menu.menut(OPCODE_LIFT, g_vm->getEngineString(S_RAISE));
}

/* NIVEAU 6 */

void tperd() {
	initouv();
	g_ment = 0;
	g_iouv = 0;
	g_mchai = 0;
	unsetSearchMenu();
	if (!g_blo) {
		int cx;
		t11(21, cx);
	}

	g_vm->_loseGame = true;
	clsf1();
	g_vm->_screenSurface.drawBox(60, 35, 400, 50, 15);
	repon(9, g_crep);
	clsf2();
	clsf3();
	g_col = false;
	g_syn = false;
	g_okdes = false;
}

void tsort() {

	if ((g_iouv > 0) && (g_s._currPlace != 0)) {
		if (g_s._faithScore < 50)
			g_s._faithScore += 2;
		else
			g_s._faithScore += (g_s._faithScore / 10);
	}

	for (int cx = 1; cx <= 7; ++cx)
		g_touv[cx] = chr(0);
	g_ment = 0;
	g_iouv = 0;
	g_mchai = 0;
	debloc(g_s._currPlace);
}

void st4(int ob) {
	g_crep = 997;

	switch (ob) {
	case 114 :
		g_crep = 109;
		break;
	case 110 :
		g_crep = 107;
		break;
	case 158 :
		g_crep = 113;
		break;
	case 152:
	case 153:
	case 154:
	case 155:
	case 156:
	case 150:
	case 100:
	case 157:
	case 160:
	case 161 :
		tlu(g_caff, ob);
		break;
	default:
		break;
	}
}

void cherjer(int ob, bool &d) {
	int cx;

	d = false;
	for (cx = 1; cx <= 6; ++cx)
		d = (d || (ord(g_s._sjer[cx]) == ob));

	if (g_s._selectedObjectId == ob)
		d = true;
}

void st1sama() {
	g_s._currPlace = DINING_ROOM;
	affrep();
}

void modinv() {
	int r;
	Common::String nomp;

	int cy = 0;
	for (int cx = 1; cx <= 6; ++cx)
		if (g_s._sjer[cx] != chr(0)) {
			++cy;
			r = (ord(g_s._sjer[cx]) + 400);
			nomp = deline(r - 501 + kInventoryStringIndex);
			g_vm->_menu.menut(g_vm->_menu._inventoryMenu[cy], nomp);
			g_vm->_menu.enableMenuItem(g_vm->_menu._inventoryMenu[cx]);
		}
	if (cy < 6)
		for (int cx = cy + 1; cx <= 6; ++cx) {
			g_vm->_menu.menut(g_vm->_menu._inventoryMenu[cx], "                       ");
			g_vm->_menu.disableMenuItem(g_vm->_menu._inventoryMenu[cx]);
		}
}

void sparl(float adr, float rep) {
	const int haut[9] = { 0, 0, 1, -3, 6, -2, 2, 7, -1 };
	int key, repint;

	repint = abs((int)rep);
	hideMouse();
	Common::String tmpStr = deline(repint + kDialogStringIndex);
	displayStr(tmpStr, 230, 4, 65, 24, 5);
	f3f8::draw();
	
	key = 0;
	do {
		parole(repint, haut[g_caff - 69], 0);
		f3f8::waitForF3F8(key);
		CHECK_QUIT;
	} while (key != 66);
	hirs();
	showMouse();
}

/**
 * Engine function - End of Search: reset globals
 * @remarks	Originally called 'finfouill'
 */
void endSearch() {
	g_heroSearching = false;
	g_obpart = false;
	g_cs = 0;
	g_is = 0;
	unsetSearchMenu();
}

/**
 * Engine function - Switch action menu to "Search" mode
 * @remarks	Originally called 'mfoudi'
 */
void setSearchMenu() {
	for (int cx = 1; cx <= 7; ++cx) 
		g_vm->_menu.disableMenuItem(g_vm->_menu._moveMenu[cx]);

	for (int cx = 1; cx <= 11; ++cx)
		g_vm->_menu.disableMenuItem(_actionMenu[cx]);

	g_vm->_menu.menut(OPCODE_SOUND, g_vm->getEngineString(S_SUITE));
	g_vm->_menu.menut(OPCODE_LIFT, g_vm->getEngineString(S_STOP));
}

void mennor() {
	g_vm->_menu.menuUp(g_msg[3]);
}

void premtet() {
	dessine(g_ades, 10, 80);
	g_vm->_screenSurface.drawBox(18, 79, 155, 91, 15);
}

/* NIVEAU 5 */
void ajchai() {
	int cy = acha + ((g_mchai - 1) * 10) - 1;
	int cx = 0;
	do {
		++cx;
	} while ((cx <= 9) && (g_tabdon[cy + cx] != 0));

	if (g_tabdon[cy + cx] == 0)
		g_tabdon[cy + cx] = g_s._selectedObjectId;
	else
		g_crep = 192;
}

void ajjer(int ob) {
	int cx = 0;
	do {
		++cx;
	} while ((cx <= 5) && (ord(g_s._sjer[cx]) != 0));

	if (ord(g_s._sjer[cx]) == 0) {
		g_s._sjer[(cx)] = chr(ob);
		modinv();
	} else
		g_crep = 139;
}

void t1sama() {    //Entering manor
	int day, hour, minute;

	updateHour(day, hour, minute);
	if ((hour < 5) && (g_s._currPlace > 18)) {
		bool d;
		cherjer(137, d);
		if (!d) {        //You don't have the keys, and it's late
			g_crep = 1511;
			tperd();
		} else
			st1sama();
	} else if (!g_s._ipre) {     //Is it your first time?
		g_ipers = 255;
		showPeoplePresent(g_ipers);
		g_caff = 77;
		afdes(0);
		g_vm->_screenSurface.drawBox(223, 47, 155, 91, 15);
		repon(2, 33);
		tkey1(false);
		mennor();
		hideMouse();
		hirs();
		premtet();
		sparl(0, 140);
		dessine_rouleau();
		drawClock();
		showMouse();
		g_s._currPlace = OWN_ROOM;
		affrep();
		t5(10);
		if (!g_blo)
			t11(0, minute);
		g_ipers = 0;
		g_mpers = 0;
		g_s._ipre = true;
	} else
		st1sama();
}

void t1vier() {
	g_s._currPlace = SECRET_PASSAGE;
	affrep();
}

void t1neig() {
	++g_inei;
	if (g_inei > 2) {
		g_crep = 1506;
		tperd();
	} else {
		g_okdes = true;
		g_s._currPlace = MOUNTAIN;
		affrep();
	}
}

void t1deva() {
	g_inei = 0;
	g_s._currPlace = MANOR_FRONT;
	affrep();
}

void t1derr() {
	g_s._currPlace = MANOR_BACK;
	affrep();
}

void t1deau() {
	g_crep = 1503;
	tperd();
}

void tctrm() {
	repon(2, (3000 + g_ctrm));
	g_ctrm = 0;
}


void quelquun() {
	if (g_imen)
		g_vm->_menu.eraseMenu();

	endSearch();
	g_crep = 997;
L1:
	if (!g_cache) {
		if (g_crep == 997)
			g_crep = 138;
		repon(2, g_crep);
		if (g_crep == 138)
			parole(5, 2, 1);
		else
			parole(4, 4, 1);

		if (g_iouv == 0)
			g_s._faithScore += 2;
		else if (g_s._faithScore < 50)
			g_s._faithScore += 4;
		else
			g_s._faithScore += 3 * (g_s._faithScore / 10);
		tsort();
		tmlieu(15);
		int cx = convertBitIndexToCharacterIndex(g_ipers);
		g_caff = 69 + cx;
		g_crep = g_caff;
		g_msg[3] = MENU_DISCUSS;
		g_msg[4] = g_vm->_menu._discussMenu[cx];
		g_syn = true;
		g_col = true;
	} else {
		if (getRandomNumber(1, 3) == 2) {
			g_cache = false;
			g_crep = 137;
			goto L1;
		} else {
			repon(2, 136);
			int rand = (getRandomNumber(0, 4)) - 2;
			parole(3, rand, 1);
			clsf2();
			displayAloneText();
			debloc(21);
			affrep();
		}
	}
	if (g_imen)
		g_vm->_menu.drawMenu();
}

void tsuiv() {
	int tbcl;
	int cl;

	int cy = acha + ((g_mchai - 1) * 10) - 1;
	int cx = 0;
	do {
		++cx;
		++g_cs;
		cl = cy + g_cs;
		tbcl = g_tabdon[cl];
	} while ((tbcl == 0) && (g_cs <= 9));

	if ((tbcl != 0) && (g_cs < 11)) {
		++g_is;
		g_caff = tbcl;
		g_crep = g_caff + 400;
		if (g_ipers != 0)
			g_s._faithScore += 2;
	} else {
		affrep();
		endSearch();
		if (cx > 9)
			g_crep = 131;
	}
}

void tfleche() {
	bool qust;
	char touch;

	if (g_num == 9999)
		return;

	fenat(chr(152));
	g_rect = false;
	do {
		touch = '\0';

		do {
			moveMouse(qust, touch);
			CHECK_QUIT;

			if (g_vm->getMouseClick())
				g_rect = (x_s < 256 * g_res) && (y_s < 176) && (y_s > 12);
			tinke();
		} while (!(qust || g_rect || g_anyone));

		if (qust && (touch == '\103'))
			Alert::show(g_hintPctMessage, 1);
	} while (!((touch == '\73') || ((touch == '\104') && (g_x != 0) && (g_y != 0)) || (g_anyone) || (g_rect)));

	if (touch == '\73')
		g_iesc = true;

	if (g_rect) {
		g_x = x_s;
		g_y = y_s;
	}
}

void tcoord(int sx) {
	int sy, ix, iy;
	int ib;


	g_num = 0;
	g_crep = 999;
	int a = 0;
	int atdon = amzon + 3;
	int cy = 0;
	while (cy < g_caff) {
		a += g_tabdon[atdon];
		atdon += 4;
		++cy;
	}

	if (g_tabdon[atdon] == 0) {
		g_crep = 997;
		return;
	}

	a += fleche;
	int cb = 0;
	for (cy = 0; cy <= (sx - 2); ++cy) {
		ib = (g_tabdon[a + cb] << 8) + g_tabdon[(a + cb + 1)];
		cb += (ib * 4) + 2;
	}
	ib = (g_tabdon[a + cb] << 8) + g_tabdon[(a + cb + 1)];
	if (ib == 0) {
		g_crep = 997;
		return;
	}

	cy = 1;
	do {
		cb += 2;
		sx = g_tabdon[a + cb] * g_res;
		sy = g_tabdon[(a + cb + 1)];
		cb += 2;
		ix = g_tabdon[a + cb] * g_res;
		iy = g_tabdon[(a + cb + 1)];
		++cy;
	} while (!(((g_x >= sx) && (g_x <= ix) && (g_y >= sy) && (g_y <= iy)) || (cy > ib)));

	if ((g_x >= sx) && (g_x <= ix) && (g_y >= sy) && (g_y <= iy)) {
		g_num = cy - 1;
		return;
	}

	g_crep = 997;
}


void st7(int ob) {
	switch (ob) {
	case 116:
	case 144:
		g_crep = 104;
		break;
	case 126:
	case 111:
		g_crep = 108;
		break;
	case 132:
		g_crep = 111;
		break;
	case 142:
		g_crep = 112;
		break;
	default:
		g_crep = 183;
		st4(ob);
	}
}

void treg(int ob) {
	int mdes = g_caff;
	g_caff = ob;

	if (((g_caff > 29) && (g_caff < 33)) || (g_caff == 144) || (g_caff == 147) || (g_caff == 149) || (g_msg[4] == OPCODE_SLOOK)) {
		afdes(0);
		if ((g_caff > 29) && (g_caff < 33))
			repon(2, g_caff);
		else
			repon(2, g_caff + 400);
		tkey1(true);
		g_caff = mdes;
		g_msg[3] = 0;
		g_crep = 998;
	} else {
		g_obpart = true;
		g_crep = g_caff + 400;
		setSearchMenu();
	}
}

void avpoing(int &ob) {
	g_crep = 999;
	if (g_s._selectedObjectId != 0)
		ajjer(g_s._selectedObjectId);

	if (g_crep != 139) {
		modobj(ob + 400);
		g_s._selectedObjectId = ob;
		ob = 0;
	}
}

void rechai(int &ch) {
	int tmpPlace = g_s._currPlace;

	if (g_s._currPlace == CRYPT)
		tmpPlace = CELLAR;
	ch = g_tabdon[achai + (tmpPlace * 7) + g_num - 1];
}

void t23coul(int &l) {
	bool d;

	cherjer(143, d);
	l = 14;
	if (!d) {
		g_crep = 1512;
		tperd();
	}
}

void maivid() {
	g_s._selectedObjectId = 0;
	modobj(500);
}

void st13(int ob) {
	if ((ob == 114) || (ob == 116) || (ob == 126) || (ob == 132) ||
	    (ob == 111) || (ob == 106) || (ob == 102) || (ob == 100) ||
	    (ob == 110) || (ob == 153) || (ob == 154) || (ob == 155) ||
	    (ob == 156) || (ob == 157) || (ob == 144) || (ob == 158) ||
	    (ob == 150) || (ob == 152))
		g_crep = 999;
	else
		g_crep = 105;
}

void aldepl() {
	Alert::show(g_vm->getEngineString(S_USE_DEP_MENU), 1);
}

} // End of namespace Mortevielle
