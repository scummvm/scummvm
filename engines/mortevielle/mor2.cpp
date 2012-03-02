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
	int cx, cf, j, h, m;
	Common::String stpo;

	anyone = false;
	calch(j, h, m);
	if (j != g_day) {
		g_day = j;
		cx = 0;
		do {
			++cx;
			if (nbrepm[cx] != 0)
				--nbrepm[cx];
			nbrep[cx] = 0;
		} while (cx != 8);
	}
	if ((h > g_hour) || ((h == 0) && (g_hour == 23))) {
		g_hour = h;
		g_minute = 0;
		drawClock();
		cf = 0;
		for (cx = 1; cx <= 10; ++cx) {
			if (g_s.pourc[cx] == '*')
				++cf;
		}

		if (cf == 10)
			stpo = "10";
		else
			stpo = chr(cf + 48);

		_hintPctMessage = Common::String(d3);
		_hintPctMessage += d5;
		_hintPctMessage += d4;
		_hintPctMessage += d3;
		_hintPctMessage += d1;
		_hintPctMessage += stpo;
		_hintPctMessage += '0';
		_hintPctMessage += d2;
		_hintPctMessage += d4;
		_hintPctMessage += d3;
		_hintPctMessage += d6;
		_hintPctMessage += d4;
	}
	if (m > g_minute) {
		g_minute = 30;
		drawClock();
	}
	if (y_s < 12)
		return;

	if (! blo) {
		if ((h == 12) || ((h > 18) && (h < 21)) || ((h >= 0) && (h < 7)))
			g_t = ti2;
		else
			g_t = ti1;
		cf = g_s.conf;
		if ((cf > 33) && (cf < 66))
			g_t -= (g_t / 3);

		if (cf > 65)
			g_t -= ((g_t / 3) * 2);

		int nh = readclock();
		if ((nh - mh) > g_t) {
			bool activeMenu = g_vm->_menu._menuActive;
			g_vm->_menu.eraseMenu();
			jh += ((nh - mh) / g_t);
			mh = nh;
			switch (li) {
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
			if ((mpers != 0) && (ipers != 10))
				mpers = ipers;

			if ((mpers == 0) && (ipers > 0)) {
				if ((g_s.mlieu == 13) || (g_s.mlieu == 14)) {
					cavegre();
				} else if (ipers == 10) {
					ipers = 0;
					if (! brt) {
						brt = true;
						hdb = readclock();
						int haz = hazard(1, 5);
						if (haz < 5) {
							clsf3();
							ecrf2();
							ecr3(g_vm->getEngineString(S_HEAR_NOISE));
							haz = (hazard(0, 4)) - 2;
							parole(1, haz, 1);
							clsf3();
						}
					}
				}
			}

			if (activeMenu)
				g_vm->_menu.drawMenu();
		}
	}
	hfb = readclock();
	if ((brt) && ((hfb - hdb) > 17)) {
		nouvp(li, cx);
		brt = false;
		hdb = 0;
		if ((g_s.mlieu > 0) && (g_s.mlieu < 10))
			anyone = true;
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
	okdes = false;
}

void tkey1(bool d) {
	bool quest;
	int x, y, c;

	hideMouse();
	fenat('K');
	while (keypressed())
		g_key = testou();

	do {
		if (d)
			tinke();
		quest = keypressed();
		getMousePos(x, y, c);
		CHECK_QUIT;
	} while (!(quest || (c != 0) || (d && anyone)));
	if (quest)
		testou();
	g_vm->setMouseClick(false);
	showMouse();
}

void tmlieu(int mli) {
	Common::String nomp;

	if (mli == 26)
		mli = 15;

	int i = 1;
	while ((i < 8) && (v_lieu[i][mli] != 0)) {
		nomp = deline(v_lieu[i][mli] + c_tmlieu);
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
	caff = 32;
	afdes(0);
	repon(6, ob + 4000);
	repon(2, 999);
	tkey1(true);
	caff = af;
	g_msg[3] = OPCODE_NONE;
	crep = 998;
}

void affrep() {
	caff = g_s.mlieu;
	crep = g_s.mlieu;
}

void mfouen() {
	tmlieu(g_s.mlieu);
	for (int cx = 1; cx <= 11; ++cx)
		g_vm->_menu.enableMenuItem(_actionMenu[cx]);

	g_vm->_menu.menut(OPCODE_SOUND, g_vm->getEngineString(S_PROBE));
	g_vm->_menu.menut(OPCODE_LIFT, g_vm->getEngineString(S_RAISE));
}

/* NIVEAU 6 */

void tperd() {
	initouv();
	ment = 0;
	iouv = 0;
	mchai = 0;
	mfouen();
	if (!blo) {
		int cx;
		t11(21, cx);
	}

	g_vm->_loseGame = true;
	clsf1();
	g_vm->_screenSurface.drawBox(60, 35, 400, 50, 15);
	repon(9, crep);
	clsf2();
	clsf3();
	g_col = false;
	syn = false;
	okdes = false;
}

void tsort() {

	if ((iouv > 0) && (g_s.mlieu != 0)) {
		if (g_s.conf < 50)
			g_s.conf += 2;
		else
			g_s.conf += (g_s.conf / 10);
	}

	for (int cx = 1; cx <= 7; ++cx)
		touv[cx] = chr(0);
	ment = 0;
	iouv = 0;
	mchai = 0;
	debloc(g_s.mlieu);
}

void st4(int ob) {
	crep = 997;

	switch (ob) {
	case 114 :
		crep = 109;
		break;
	case 110 :
		crep = 107;
		break;
	case 158 :
		crep = 113;
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
		tlu(caff, ob);
		break;
	default:
		break;
	}
}

void cherjer(int ob, bool &d) {
	int cx;

	d = false;
	for (cx = 1; cx <= 6; ++cx)
		d = (d || (ord(g_s.sjer[cx]) == ob));

	if (g_s.derobj == ob)
		d = true;
}

void st1sama() {
	g_s.mlieu = 10;
	affrep();
}

void modinv() {
	int r;
	Common::String nomp;

	int cy = 0;
	for (int cx = 1; cx <= 6; ++cx)
		if (g_s.sjer[cx] != chr(0)) {
			++cy;
			r = (ord(g_s.sjer[cx]) + 400);
			nomp = deline(r - 501 + c_st41);
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
	Common::String tmpStr = deline(repint + c_paroles);
	afftex(tmpStr, 230, 4, 65, 24, 5);
	f3f8::draw();
	
	key = 0;
	do {
		parole(repint, haut[caff - 69], 0);
		f3f8::waitForF3F8(key);
		CHECK_QUIT;
	} while (key != 66);
	hirs();
	showMouse();
}

void finfouil() {
	fouil = false;
	obpart = false;
	cs = 0;
	is = 0;
	mfouen();
}

void mfoudi() {
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
	dessine(ades, 10, 80);
	g_vm->_screenSurface.drawBox(18, 79, 155, 91, 15);
}

/* NIVEAU 5 */
void ajchai() {
	int cy = acha + ((mchai - 1) * 10) - 1;
	int cx = 0;
	do {
		++cx;
	} while ((cx <= 9) && (tabdon[cy + cx] != 0));

	if (tabdon[cy + cx] == 0) {
		int lderobj = g_s.derobj;
		tabdon[cy + cx] = lderobj;
	} else
		crep = 192;
}

void ajjer(int ob) {
	int cx = 0;
	do {
		++cx;
	} while ((cx <= 5) && (ord(g_s.sjer[cx]) != 0));

	if (ord(g_s.sjer[cx]) == 0) {
		g_s.sjer[(cx)] = chr(ob);
		modinv();
	} else
		crep = 139;
}

void t1sama() {    //Entering manor
	int j, h, m;

	calch(j, h, m);
	if ((h < 5) && (g_s.mlieu > 18)) {
		bool d;
		cherjer(137, d);
		if (!d) {        //You don't have the keys, and it's late
			crep = 1511;
			tperd();
		} else
			st1sama();
	} else if (!g_s.ipre) {     //Is it your first time?
		ipers = 255;
		affper(ipers);
		caff = 77;
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
		g_s.mlieu = 0;
		affrep();
		t5(10);
		if (! blo)
			t11(0, m);
		ipers = 0;
		mpers = 0;
		g_s.ipre = true;
		/*chech;*/
	} else
		st1sama();
}

void t1vier() {
	g_s.mlieu = 17;
	affrep();
}

void t1neig() {
	++inei;
	if (inei > 2) {
		crep = 1506;
		tperd();
	} else {
		okdes = true;
		g_s.mlieu = 19;
		affrep();
	}
}

void t1deva() {
	inei = 0;
	g_s.mlieu = 21;
	affrep();
}

void t1derr() {
	g_s.mlieu = 22;
	affrep();
}

void t1deau() {
	crep = 1503;
	tperd();
}

void tctrm() {
	repon(2, (3000 + ctrm));
	ctrm = 0;
}


void quelquun() {
	if (imen)
		g_vm->_menu.eraseMenu();

	finfouil();
	crep = 997;
L1:
	if (! cache) {
		if (crep == 997)
			crep = 138;
		repon(2, crep);
		if (crep == 138)
			parole(5, 2, 1);
		else
			parole(4, 4, 1);

		if (iouv == 0)
			g_s.conf += 2;
		else if (g_s.conf < 50)
			g_s.conf += 4;
		else
			g_s.conf += 3 * (g_s.conf / 10);
		tsort();
		tmlieu(15);
		int cx;
		tip(ipers, cx);
		caff = 69 + cx;
		crep = caff;
		g_msg[3] = MENU_DISCUSS;
		g_msg[4] = g_vm->_menu._discussMenu[cx];
		syn = true;
		g_col = true;
	} else {
		int haz = hazard(1, 3);
		if (haz == 2) {
			cache = false;
			crep = 137;
			goto L1;
		} else {
			repon(2, 136);
			haz = (hazard(0, 4)) - 2;
			parole(3, haz, 1);
			clsf2();
			person();
			debloc(21);
			affrep();
		}
	}
	if (imen)
		g_vm->_menu.drawMenu();
}

void tsuiv() {
	int tbcl;
	int cl;

	int cy = acha + ((mchai - 1) * 10) - 1;
	int cx = 0;
	do {
		cx = cx + 1;
		cs = cs + 1;
		cl = cy + cs;
		tbcl = tabdon[cl];
	} while ((tbcl == 0) && (cs <= 9));

	if ((tbcl != 0) && (cs < 11)) {
		is = is + 1;
		caff = tbcl;
		crep = caff + 400;
		if (ipers != 0)
			g_s.conf = g_s.conf + 2;
	} else {
		affrep();
		finfouil();
		if (cx > 9)
			crep = 131;
	}
}

void tfleche() {
	bool qust;
	char touch;

	if (g_num == 9999)
		return;

	fenat(chr(152));
	rect = false;
	do {
		touch = '\0';

		do {
			moveMouse(qust, touch);
			CHECK_QUIT;

			if (g_vm->getMouseClick())
				rect = (x_s < 256 * res) && (y_s < 176) && (y_s > 12);
			tinke();
		} while (!(qust || rect || anyone));

		if (qust && (touch == '\103'))
			Alert::show(_hintPctMessage, 1);
	} while (!((touch == '\73') || ((touch == '\104') && (g_x != 0) && (g_y != 0)) || (anyone) || (rect)));

	if (touch == '\73')
		iesc = true;

	if (rect) {
		g_x = x_s;
		g_y = y_s;
	}
}

void tcoord(int sx) {
	int sy, ix, iy;
	int ib;


	g_num = 0;
	crep = 999;
	int a = 0;
	int atdon = amzon + 3;
	int cy = 0;
	while (cy < caff) {
		a += tabdon[atdon];
		atdon += 4;
		++cy;
	}

	if (tabdon[atdon] == 0) {
		crep = 997;
		return;
	}

	a += fleche;
	int cb = 0;
	for (cy = 0; cy <= (sx - 2); ++cy) {
		ib = (tabdon[a + cb] << 8) + tabdon[(a + cb + 1)];
		cb += (ib * 4) + 2;
	}
	ib = (tabdon[a + cb] << 8) + tabdon[(a + cb + 1)];
	if (ib == 0) {
		crep = 997;
		return;
	}

	cy = 1;
	do {
		cb += 2;
		sx = tabdon[a + cb] * res;
		sy = tabdon[(a + cb + 1)];
		cb += 2;
		ix = tabdon[a + cb] * res;
		iy = tabdon[(a + cb + 1)];
		++cy;
	} while (!(((g_x >= sx) && (g_x <= ix) && (g_y >= sy) && (g_y <= iy)) || (cy > ib)));

	if ((g_x >= sx) && (g_x <= ix) && (g_y >= sy) && (g_y <= iy)) {
		g_num = cy - 1;
		return;
	}

	crep = 997;
}


void st7(int ob) {
	switch (ob) {
	case 116:
	case 144:
		crep = 104;
		break;
	case 126:
	case 111:
		crep = 108;
		break;
	case 132:
		crep = 111;
		break;
	case 142:
		crep = 112;
		break;
	default:
		crep = 183;
		st4(ob);
	}
}

void treg(int ob) {
	int mdes = caff;
	caff = ob;

	if (((caff > 29) && (caff < 33)) || (caff == 144) || (caff == 147) || (caff == 149) || (g_msg[4] == OPCODE_SLOOK)) {
		afdes(0);
		if ((caff > 29) && (caff < 33))
			repon(2, caff);
		else
			repon(2, (caff + 400));
		tkey1(true);
		caff = mdes;
		g_msg[3] = 0;
		crep = 998;
	} else {
		obpart = true;
		crep = caff + 400;
		mfoudi();
	}
}

void avpoing(int &ob) {
	crep = 999;
	if (g_s.derobj != 0)
		ajjer(g_s.derobj);

	if (crep != 139) {
		modobj(ob + 400);
		g_s.derobj = ob;
		ob = 0;
	}
}

void rechai(int &ch) {
	int cx = g_s.mlieu;

	if (g_s.mlieu == 16)
		cx = 14;
	ch = tabdon[achai + (cx * 7) + g_num - 1];
}

void t23coul(int &l) {
	bool d;

	cherjer(143, d);
	l = 14;
	if (!d) {
		crep = 1512;
		tperd();
	}
}

void maivid() {
	g_s.derobj = 0;
	modobj(500);
}

void st13(int ob) {
	if ((ob == 114) || (ob == 116) || (ob == 126) || (ob == 132) ||
	    (ob == 111) || (ob == 106) || (ob == 102) || (ob == 100) ||
	    (ob == 110) || (ob == 153) || (ob == 154) || (ob == 155) ||
	    (ob == 156) || (ob == 157) || (ob == 144) || (ob == 158) ||
	    (ob == 150) || (ob == 152))
		crep = 999;
	else
		crep = 105;
}

void aldepl() {
	Alert::show(g_vm->getEngineString(S_USE_DEP_MENU), 1);
}

} // End of namespace Mortevielle
