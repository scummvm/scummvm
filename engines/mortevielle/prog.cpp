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

#include "mortevielle/actions.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/menu.h"
#include "mortevielle/mor.h"
#include "mortevielle/mor2.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/prog.h"
#include "mortevielle/taffich.h"
#include "mortevielle/var_mor.h"
#include "mortevielle/mouse.h"

namespace Mortevielle {

void change_gd(int ngd) {
	hideMouse();
	gd = ngd;
	hirs();
	initMouse();
	showMouse();
	dessine_rouleau();
	tinke();
	drawClock();
	if (ipers != 0)
		affper(ipers);
	else
		person();
	clsf2();
	clsf3();
	_maff = 68;
	afdes(0);
	repon(2, crep);
	g_vm->_menu.menu_aff();
}

/**
 * Called when a savegame has been loaded.
 * @remarks	Originally called 'antegame'
 */
void MortevielleEngine::gameLoaded() {
	int k;
	bool test[3];
	char g[8];

	hideMouse();
	imen = false;
	g[1] = 'M';
	_loseGame = true;
	anyone = false;
	okdes = true;
	test[0] = false;
	test[1] = false;
	g[0] = '\040';
	col = false;
	tesok = true;
	test[2] = false;
	g[7] = g[0];
	g[2] = 'A';
	cache = false;
	brt = false;
	_maff = 68;
	g[5] = 'E';
	mnumo = 0;
	prebru = 0;
	g[4] = 'T';
	x = 0;
	y = 0;
	num = 0;
	hdb = 0;
	hfb = 0;
	cs = 0;
	is = 0;
	k = 0;
	ment = 0;
	syn = true;
	fouil = true;
	mchai = 0;
	inei = 0;
	initouv();
	g[3] = 'S';
	g[6] = 'R';
	iouv = 0;
	dobj = 0;
	affrep();
	stpou = ind_mess;
	while ((test[k] == false) && (k < 2)) {
		++k;

		// Original read in the boot sector here of each drive and did some calculations -
		// presumably for copy protection. This has been replaced with hardcoded success
		test[k] = true;

		okdes = false;
		_endGame = true;
		_loseGame = false;
		fouil = false;
	}
	person();
	tinke();
	drawClock();
	afdes(0);
	repon(2, crep);
	clsf3();
	_endGame = false;
	tmlieu(s.mlieu);
	modinv();
	if (s.derobj != 0)
		modobj2(s.derobj + 400, test[1], test[2]);
	else
		tesok = test[1] || test[2];
	showMouse();
}


/* NIVEAU 3 */
/* procedure PROGRAMME */
void tmaj3() {
	int j, h, m;

	calch(j, h, m);
	if (m == 30)
		m = 1;
	h += j * 24;
	m += h * 2;
	s.heure = chr(m);
}

void tsitu() {
	int h, j, m;

	if (! col)
		clsf2();
	syn = false;
	iesc = false;
	if (anyone)
		goto L1;
	if (brt)
		if ((msg[3] == MENU_MOVE) || (msg[4] == OPCODE_LEAVE) || (msg[4] == OPCODE_SLEEP) || (msg[4] == OPCODE_EAT)) {
			ctrm = 4;
			goto L2;
		}
	if (msg[3] == MENU_MOVE)
		fctMove();
	if (msg[3] == MENU_DISCUSS)
		fctDiscuss();
	if (msg[3] == invent)
		tsprendre(); //Translation: inventory/take
	if (msg[4] == OPCODE_ATTACH)
		fctAttach();
	if (msg[4] == OPCODE_WAIT)
		fctWait();
	if (msg[4] == OPCODE_FORCE)
		fctForce();
	if (msg[4] == OPCODE_SLEEP)
		fctSleep();
	if (msg[4] == OPCODE_LISTEN)
		fctListen();
	if (msg[4] == OPCODE_ENTER)
		fctEnter();
	if (msg[4] == OPCODE_CLOSE)
		fctClose();
	if (msg[4] == OPCODE_SEARCH)
		fctSearch();
	if (msg[4] == OPCODE_KNOCK)
		fctKnock();
	if (msg[4] == OPCODE_SCRATCH)
		fctScratch();
	if (msg[4] == OPCODE_READ)
		fctRead();
	if (msg[4] == OPCODE_EAT)
		fctEat();
	if (msg[4] == OPCODE_PLACE)
		fctPlace();
	if (msg[4] == OPCODE_OPEN)
		fctOpen();
	if (msg[4] == OPCODE_TAKE)
		fctTake();
	if (msg[4] == OPCODE_LOOK)
		fctLook();
	if (msg[4] == OPCODE_SMELL)
		fctSmell();
	if (msg[4] == OPCODE_SOUND)
		fctSound();
	if (msg[4] == OPCODE_LEAVE)
		fctLeave();
	if (msg[4] == OPCODE_LIFT)
		fctLift();
	if (msg[4] == OPCODE_TURN)
		fctTurn();
	if (msg[4] == OPCODE_HIDE_SELF) {
		fctHideSelf();
		goto L1;
	}
	if (msg[4] == sfouiller)
		tsfouiller();//Translation: search 
	if (msg[4] == slire)
		tslire();    //Translation: read
	if (msg[4] == OPCODE_PUT)
		fctPut();
	if (msg[4] == sregarder)
		tsregarder();//Translation: look
	cache = false;
L1:
	if (anyone) {
		quelquun();
		anyone = false;
		goto L2;
	}
	calch(j, h, m);
	if ((((h == 12) || (h == 13) || (h == 19)) && (s.mlieu != 10)) ||
	        ((h > 0) && (h < 6) && (s.mlieu != 0)))
		s.conf = s.conf + 1;
	if (((s.mlieu < 16) || (s.mlieu > 19)) && (s.mlieu != 23)
	        && (s.mlieu != 0) && (s.derobj != 152) && (!g_vm->_loseGame)) {
		if ((s.conf > 99) && (h > 8) && (h < 16)) {
			crep = 1501;
			tperd();
		}
		if ((s.conf > 99) && (h > 0) && (h < 9)) {
			crep = 1508;
			tperd();
		}
		if ((j > 1) && (h > 8) && (!g_vm->_loseGame)) {
			crep = 1502;
			tperd();
		}
	}
L2:
	mennor();
}

/* NIVEAU 1 */

void theure() {
	vj = ord(s.heure);
	vh = vj % 48;
	vj /= 48;
	vm = vh % 2;
	vh /= 2;
	_hour = vh;
	if (vm == 1)
		_minute = 30;
	else
		_minute = 0;
}

} // End of namespace Mortevielle
