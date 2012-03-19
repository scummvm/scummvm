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

#include "common/endian.h"
#include "common/file.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/graphics.h"
#include "mortevielle/menu.h"
#include "mortevielle/mor.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/speech.h"
#include "mortevielle/var_mor.h"

#include "mortevielle/keyboard.h"

namespace Mortevielle {

/* NIVEAU 15 */
void copcha() {
	int i = acha;
	do {
		g_tabdon[i] = g_tabdon[i + 390];
		++i;
	} while (i != acha + 390);
}

void outbloc(int n, pattern p, nhom *pal) {
	int ad = n * 404 + 0xd700;

	WRITE_LE_UINT16(&g_mem[0x6000 * 16 + ad], p._tax);
	WRITE_LE_UINT16(&g_mem[0x6000 * 16 + ad + 2], p._tay);
	ad += 4;
	for (int i = 1; i <= p._tax; ++i) {
		for (int j = 1; j <= p._tay; ++j)
			g_mem[(0x6000 * 16) + ad + (j - 1) * p._tax + i - 1] = pal[n]._hom[p._des[i][j]];
	}
}

void writepal(int n) {
	switch (g_vm->_currGraphicalDevice) {
	case MODE_TANDY:
	case MODE_EGA:
	case MODE_AMSTRAD1512:
		for (int i = 1; i <= 16; ++i) {
			g_mem[(0x7000 * 16) + (2 * i)] = g_tabpal[n][i].x;
			g_mem[(0x7000 * 16) + (2 * i) + 1] = g_tabpal[n][i].y;
		}
		break;
	case MODE_CGA: {
		nhom pal[16];
		for (int i = 0; i < 16; ++i) {
			pal[i] = g_palcga[n]._a[i];
		}

		if (n < 89)
			palette(g_palcga[n]._p);
		
		for (int i = 0; i <= 15; ++i)
			outbloc(i, g_tpt[pal[i]._id], pal);
		}
		break;
	default:
		break;
	}
}


void pictout(int seg, int dep, int x, int y) {
	GfxSurface surface;
	surface.decode(&g_mem[seg * 16 + dep]);

	if (g_vm->_currGraphicalDevice == MODE_HERCULES) {
		g_mem[0x7000 * 16 + 2] = 0;
		g_mem[0x7000 * 16 + 32] = 15;
	}

	if ((g_caff != 51) && (READ_LE_UINT16(&g_mem[0x7000 * 16 + 0x4138]) > 0x100))
		WRITE_LE_UINT16(&g_mem[0x7000 * 16 + 0x4138], 0x100);

	g_vm->_screenSurface.drawPicture(surface, x, y);
}

void sauvecr(int y, int dy) {
	g_vm->_mouse.hideMouse();
	s_sauv(g_vm->_currGraphicalDevice, y, dy);
	g_vm->_mouse.showMouse();
}

void charecr(int y, int dy) {
	g_vm->_mouse.hideMouse();
	s_char(g_vm->_currGraphicalDevice, y, dy);
	g_vm->_mouse.showMouse();
}

void adzon() {
	Common::File f;

	if (!f.open("don.mor"))
		error("Missing file - don.mor");

	f.read(g_tabdon, 7 * 256);
	f.close();

	if (!f.open("bmor.mor"))
		error("Missing file - bmor.mor");

	f.read(&g_tabdon[fleche], 1 * 1916);
	f.close();

	if (!f.open("dec.mor"))
		error("Missing file - dec.mor");

	f.read(&g_mem[0x73a2 * 16 + 0], 1 * 1664);
	f.close();
}

/**
 * Returns the offset within the compressed image data resource of the desired image
 */
int animof(int ouf, int num) {
	int nani = g_mem[kAdrAni * 16 + 1];
	int aux = num;
	if (ouf != 1)
		aux += nani;

	int animof_result = (nani << 2) + 2 + READ_BE_UINT16(&g_mem[kAdrAni * 16 + (aux << 1)]);

	return animof_result;
}

/**
 * Read the current system time
 */
int readclock() {
	TimeDate dateTime;
	g_system->getTimeAndDate(dateTime);

	int m = dateTime.tm_min * 60;
	int h = dateTime.tm_hour * 3600;
	return h + m + dateTime.tm_sec;
}

void modif(int &nu) {
	if (nu == 26)
		nu = 25;
	else if ((nu > 29) && (nu < 36))
		nu -= 4;
	else if ((nu > 69) && (nu < 78))
		nu -= 37;
	else if ((nu > 99) && (nu < 194))
		nu -= 59;
	else if ((nu > 996) && (nu < 1000))
		nu -= 862;
	else if ((nu > 1500) && (nu < 1507))
		nu -= 1363;
	else if ((nu > 1507) && (nu < 1513))
		nu -= 1364;
	else if ((nu > 1999) && (nu < 2002))
		nu -= 1851;
	else if (nu == 2010)
		nu = 151;
	else if ((nu > 2011) && (nu < 2025))
		nu -= 1860;
	else if (nu == 2026)
		nu = 165;
	else if ((nu > 2029) && (nu < 2037))
		nu -= 1864;
	else if ((nu > 3000) && (nu < 3005))
		nu -= 2828;
	else if (nu == 4100)
		nu = 177;
	else if (nu == 4150)
		nu = 178;
	else if ((nu > 4151) && (nu < 4156))
		nu -= 3973;
	else if (nu == 4157)
		nu = 183;
	else if ((nu == 4160) || (nu == 4161))
		nu -= 3976;
}


void dessine(int ad, int x, int y) {
	g_vm->_mouse.hideMouse();
	writepal(g_numpal);
	pictout(ad, 0, x, y);
	g_vm->_mouse.showMouse();
}

/**
 * Draw right frame
 * @remarks	Originally called 'dessine_rouleau'
 */
void drawRightFrame() {
	writepal(89);
	if (g_vm->_currGraphicalDevice == MODE_HERCULES) {
		g_mem[0x7000 * 16 + 14] = 15;
	}
	g_vm->_mouse.hideMouse();
	pictout(0x73a2, 0, 0, 0);
	g_vm->_mouse.showMouse();
}

/* NIVEAU 13 */


void text1(int x, int y, int nb, int m) {
	int co;

	if (g_res == 1)
		co = 10;
	else
		co = 6;
	Common::String tmpStr = deline(m);
	if ((y == 182) && ((int) tmpStr.size() * co > nb * 6))
		y = 176;
	displayStr(tmpStr, x, y, nb, 20, g_vm->_textColor);
}

void initouv() {
	for (int cx = 1; cx <= 7; ++cx)
		g_touv[cx] = chr(0);
}

void ecr2(Common::String str_) {
	// Some dead code was present in the original: removed
	g_vm->_screenSurface.putxy(8, 177);
	int tlig = 59 + (g_res - 1) * 36;

	if ((int)str_.size() < tlig)
		g_vm->_screenSurface.drawString(str_, 5);
	else if ((int)str_.size() < (tlig << 1)) {
		g_vm->_screenSurface.putxy(8, 176);
		g_vm->_screenSurface.drawString(copy(str_, 1, (tlig - 1)), 5);
		g_vm->_screenSurface.putxy(8, 182);
		g_vm->_screenSurface.drawString(copy(str_, tlig, tlig << 1), 5);
	} else {
		g_vm->_largestClearScreen = true;
		g_vm->clearScreenType2();
		g_vm->_screenSurface.putxy(8, 176);
		g_vm->_screenSurface.drawString(copy(str_, 1, (tlig - 1)), 5);
		g_vm->_screenSurface.putxy(8, 182);
		g_vm->_screenSurface.drawString(copy(str_, tlig, ((tlig << 1) - 1)), 5);
		g_vm->_screenSurface.putxy(8, 190);
		g_vm->_screenSurface.drawString(copy(str_, tlig << 1, tlig * 3), 5);
	}
}

void ecr3(Common::String text) {
	g_vm->clearScreenType3();
	g_vm->_screenSurface.putxy(8, 192);
	g_vm->_screenSurface.drawString(text, 5);
}

void stop() {
	hirs();
	g_vm->_currGraphicalDevice = MODE_AMSTRAD1512;
	hirs();
	g_vm->quitGame();
}

void paint_rect(int x, int y, int dx, int dy) {
	int co;

	if (g_vm->_currGraphicalDevice == MODE_CGA)
		co = 3;
	else
		co = 11;
	g_vm->_screenSurface.fillRect(co, Common::Rect(x, y, x + dx, y + dy));
}

/* NIVEAU 12 */
void modobj(int m) {
	Common::String strp = Common::String(' ');

	if (m != 500)
		strp = deline(m - 501 + kInventoryStringIndex);

	g_vm->_menu.setText(g_vm->_menu._inventoryMenu[8], strp);
	g_vm->_menu.disableMenuItem(g_vm->_menu._inventoryMenu[8]);
}

void repon(int f, int m) {
	if ((m > 499) && (m < 563)) {
		Common::String tmpStr = deline(m - 501 + kInventoryStringIndex);

		if ((int) tmpStr.size() > ((58 + (g_res - 1) * 37) << 1))
			g_vm->_largestClearScreen = true;
		else
			g_vm->_largestClearScreen = false;

		g_vm->clearScreenType2();
		displayStr(tmpStr, 8, 176, 85, 3, 5);
	} else {
		modif(m);
		switch (f) {
		case 2:
		case 8:
			g_vm->clearScreenType2();
			g_vm->prepareScreenType2();
			text1(8, 182, 103, m);
			if ((m == 68) || (m == 69))
				g_s._teauto[40] = '*';
			if ((m == 104) && (g_caff == 14)) {
				g_s._teauto[36] = '*';
				if (g_s._teauto[39] == '*') {
					g_s._pourc[3] = '*';
					g_s._teauto[38] = '*';
				}
			}
			break;
		case 1:
		case 6:
		case 9: {
			int i;
			if ((f == 1) || (f == 6))
				i = 4;
			else
				i = 5;

			Common::String tmpStr = deline(m);
			displayStr(tmpStr, 80, 40, 60, 25, i);

			if (m == 180)
				g_s._pourc[6] = '*';
			else if (m == 179)
				g_s._pourc[10] = '*';
			}
			break;
		default:
			break;
		}
	}
}

int chlm() {
	int retval = getRandomNumber(1, 2);
	if (retval == 2)
		retval = 128;
	
	return retval;
}

/**
 * Engine function - Draw Clock
 * @remarks	Originally called 'pendule'
 */
void drawClock() {
	const int cv[2][12] = {
		{  5,  8, 10,  8,  5,  0, -5, -8, -10, -8, -5,  0 },
		{ -5, -3,  0,  3,  5,  6,  5,  3,   0, -3, -5, -6 }
	};
	const int x = 580;
	const int y = 123;
	const int rg = 9;
	int h, co;

	g_vm->_mouse.hideMouse();
	
	paint_rect(570, 118, 20, 10);
	paint_rect(578, 114, 6, 18);
	if ((g_vm->_currGraphicalDevice == MODE_CGA) || (g_vm->_currGraphicalDevice == MODE_HERCULES))
		co = 0;
	else
		co = 1;

	if (g_vm->_minute == 0)
		g_vm->_screenSurface.drawLine(((uint)x >> 1) * g_res, y, ((uint)x >> 1) * g_res, (y - rg), co);
	else 
		g_vm->_screenSurface.drawLine(((uint)x >> 1) * g_res, y, ((uint)x >> 1) * g_res, (y + rg), co);

	h = g_vm->_hour;
	if (h > 12)
		h -= 12;
	if (h == 0)
		h = 12;

	g_vm->_screenSurface.drawLine(((uint)x >> 1) * g_res, y, ((uint)(x + cv[0][h - 1]) >> 1) * g_res, y + cv[1][h - 1], co);
	g_vm->_mouse.showMouse();
	g_vm->_screenSurface.putxy(568, 154);

	if (g_vm->_hour > 11)
		g_vm->_screenSurface.drawString("PM ", 1);
	else
		g_vm->_screenSurface.drawString("AM ", 1);

	g_vm->_screenSurface.putxy(550, 160);
	if ((g_vm->_day >= 0) && (g_vm->_day <= 8)) {
		Common::String tmp = g_vm->getEngineString(S_DAY);
		tmp.insertChar((char)(g_vm->_day + 49), 0);
		g_vm->_screenSurface.drawString(tmp, 1);
	}
}

/*************
 * NIVEAU 11 *
 *************/

void debloc(int roomId) {
	g_num = 0;
	g_x = 0;
	g_y = 0;
	if ((roomId != ROOM26) && (roomId != LANDING))
		g_vm->resetPresenceInRooms(roomId);
	g_mpers = g_vm->_currBitIndex;
}

void ecfren(int &p, int &rand, int cf, int roomId) {
	if (roomId == OWN_ROOM)
		g_vm->displayAloneText();
	p = -500;
	rand = 0;
	if ( ((roomId == GREEN_ROOM) && (!g_vm->_roomPresenceLuc) && (!g_vm->_roomPresenceIda))
	  || ((roomId == DARKBLUE_ROOM) && (!g_vm->_roomPresenceGuy) && (!g_vm->_roomPresenceEva)) )
		p = g_vm->getPresenceStatsGreenRoom();
	if ((roomId == PURPLE_ROOM) && (!g_vm->_purpleRoomPresenceLeo) && (!g_vm->_room9PresenceLeo))
		p = g_vm->getPresenceStatsPurpleRoom();
	if ( ((roomId == TOILETS) && (!g_vm->_toiletsPresenceBobMax))
	  || ((roomId == BATHROOM) && (!g_vm->_bathRoomPresenceBobMax)) )
		p = g_vm->getPresenceStatsToilets();
	if ((roomId == BLUE_ROOM) && (!g_vm->_roomPresenceMax))
		p = g_vm->getPresenceStatsBlueRoom();
	if ( ((roomId == RED_ROOM) && (!g_vm->_roomPresenceBob))
	  || ((roomId == GREEN_ROOM2) && (!g_vm->_roomPresencePat)))
		p = g_vm->getPresenceStatsRedRoom();
	if ((roomId == ROOM9) && (!g_vm->_room9PresenceLeo) && (!g_vm->_purpleRoomPresenceLeo))
		p = 10;
	if ( ((roomId == PURPLE_ROOM) && (g_vm->_room9PresenceLeo))
	  || ((roomId == ROOM9) && (g_vm->_purpleRoomPresenceLeo)))
		p = -400;
	if (p != -500) {
		p += cf;
		rand = getRandomNumber(1, 100);
	}
}

void becfren(int roomId) {
	if ((roomId == GREEN_ROOM) || (roomId == DARKBLUE_ROOM)) {
		int rand = getRandomNumber(1, 2);
		if (roomId == GREEN_ROOM) {
			if (rand == 1)
				g_vm->_roomPresenceLuc = true;
			else
				g_vm->_roomPresenceIda = true;
		} else { // roomId == DARKBLUE_ROOM
			if (rand == 1)
				g_vm->_roomPresenceGuy = true;
			else
				g_vm->_roomPresenceEva = true;
		}
	} else if (roomId == PURPLE_ROOM)
		g_vm->_purpleRoomPresenceLeo = true;
	else if (roomId == TOILETS)
		g_vm->_toiletsPresenceBobMax = true;
	else if (roomId == BLUE_ROOM)
		g_vm->_roomPresenceMax = true;
	else if (roomId == RED_ROOM)
		g_vm->_roomPresenceBob = true;
	else if (roomId == BATHROOM)
		g_vm->_bathRoomPresenceBobMax = true;
	else if (roomId == GREEN_ROOM2)
		g_vm->_roomPresencePat = true;
	else if (roomId == ROOM9)
		g_vm->_room9PresenceLeo = true;
}

/* NIVEAU 10 */
void init_nbrepm() {
	const byte ipm[9] = { 0, 4, 5, 6, 7, 5, 6, 5, 8 };

	for (int idx = 0; idx < 9; ++idx)
		g_nbrepm[idx] = ipm[idx];
}

void phaz(int &rand, int &p, int cf) {
	p += cf;
	rand = getRandomNumber(1, 100);
}

/**
 * Engine function - When restarting the game, reset the main variables used by the engine
 * @remarks	Originally called 'inzon'
 */
void resetVariables() {
	copcha();

	g_s._alreadyEnteredManor = false;
	g_s._selectedObjectId = 0;
	g_s._cellarObjectId = 0;
	g_s._atticBallHoleObjectId = 0;
	g_s._atticRodHoleObjectId = 0;
	g_s._wellObjectId = 0;
	g_s._secretPassageObjectId = 0;
	g_s._purpleRoomObjectId = 136;
	g_s._cryptObjectId = 141;
	g_s._faithScore = getRandomNumber(4, 10);
	g_s._currPlace = MANOR_FRONT;

	for (int i = 2; i <= 6; ++i)
		g_s._sjer[i] = chr(0);

	g_s._sjer[1] = chr(113);
	g_s._fullHour = chr(20);

	for (int i = 1; i <= 10; ++i)
		g_s._pourc[i] = ' ';

	for (int i = 1; i <= 6; ++i)
		g_s._teauto[i] = '*';

	for (int i = 7; i <= 9; ++i)
		g_s._teauto[i] = ' ';

	for (int i = 10; i <= 28; ++i)
		g_s._teauto[i] = '*';

	for (int i = 29; i <= 42; ++i)
		g_s._teauto[i] = ' ';

	g_s._teauto[33] = '*';

	for (int i = 1; i <= 8; ++i)
		g_nbrep[i] = 0;

	init_nbrepm();
}

int t11(int roomId) {
	int retVal = 0;
	int p, rand;

	ecfren(p, rand, g_s._faithScore, roomId);
	g_vm->_place = roomId;
	if ((roomId > OWN_ROOM) && (roomId < DINING_ROOM)) {
		if (p != -500) {
			if (rand > p) {
				g_vm->displayAloneText();
				retVal = 0;
			} else {
				becfren(g_vm->_place);
				retVal = g_vm->getPresenceBitIndex(g_vm->_place);
			}
		} else
			retVal = g_vm->getPresenceBitIndex(g_vm->_place);
	}

	if (roomId > ROOM9) {
		if ((roomId > LANDING) && (roomId != CHAPEL) && (roomId != ROOM26))
			g_vm->displayAloneText();
		else {
			int h = 0;
			if (roomId == DINING_ROOM)
				p = g_vm->getPresenceStatsDiningRoom(h);
			else if (roomId == BUREAU)
				p = g_vm->getPresenceStatsBureau(h);
			else if (roomId == KITCHEN)
				p = g_vm->getPresenceStatsKitchen();
			else if ((roomId == ATTIC) || (roomId == CELLAR))
				p = g_vm->getPresenceStatsAttic();
			else if ((roomId == LANDING) || (roomId == ROOM26))
				p = g_vm->getPresenceStatsLanding();
			else if (roomId == CHAPEL)
				p = g_vm->getPresenceStatsChapel(h);
			p += g_s._faithScore;
			rand = getRandomNumber(1, 100);
			if (rand > p) {
				g_vm->displayAloneText();
				retVal = 0;
			} else {
				if (roomId == DINING_ROOM)
					p = g_vm->setPresenceDiningRoom(h);
				else if (roomId == BUREAU)
					p = g_vm->setPresenceBureau(h);
				else if ((roomId == KITCHEN) || (roomId == ATTIC) || (roomId == CELLAR))
					p = g_vm->setPresenceKitchen();
				else if ((roomId == LANDING) || (roomId == ROOM26))
					p = g_vm->setPresenceLanding();
				else if (roomId == CHAPEL)
					p = g_vm->setPresenceChapel(h);
				retVal = p;
			}
		}
	}

	return retVal;
}

void cavegre() {
	g_s._faithScore += 2;
	if (g_s._faithScore > 69)
		g_s._faithScore += (g_s._faithScore / 10);
	g_vm->clearScreenType3();
	g_vm->prepareScreenType2();
	ecr3(g_vm->getEngineString(S_SOMEONE_ENTERS));
	int rand = (getRandomNumber(0, 4)) - 2;
	g_vm->_speechManager.startSpeech(2, rand, 1);

	// The original was doing here a useless loop.
	// It has been removed

	g_vm->clearScreenType3();
	g_vm->displayAloneText();
}

void writetp(Common::String s, int t) {
	if (g_res == 2)
		g_vm->_screenSurface.drawString(s, t);
	else
		g_vm->_screenSurface.drawString(copy(s, 1, 25), t);
}

void aniof(int ouf, int num) {
	if ((g_caff == 7) && ((num == 4) || (num == 5)))
		return;
	
	if ((g_caff == 10) && (num == 7))
		num = 6;
	else if (g_caff == 12) {
		if (num == 3)
			num = 4;
		else if (num == 4)
			num = 3;
	}

	int ad = kAdrAni;
	int offset = animof(ouf, num);

	GfxSurface surface;
	surface.decode(&g_mem[ad * 16 + offset]);
	g_vm->_screenSurface.drawPicture(surface, 0, 12);

	g_vm->prepareScreenType1();
}

/* NIVEAU 9 */
void dessin(int ad) {
	if (ad != 0)
		dessine(g_ades, ((ad % 160) * 2), (ad / 160));
	else {
		g_vm->clearScreenType1();
		if (g_caff > 99) {
			dessine(g_ades, 60, 33);
			g_vm->_screenSurface.drawBox(118, 32, 291, 121, 15);         // Medium box
		} else if (g_caff > 69) {
			dessine(g_ades, 112, 48);           // Heads
			g_vm->_screenSurface.drawBox(222, 47, 155, 91, 15);
		} else {
			dessine(g_ades, 0, 12);
			g_vm->prepareScreenType1();
			if ((g_caff < 30) || (g_caff > 32)) {
				for (int cx = 1; cx <= 6; ++cx) {
					if (ord(g_touv[cx]) != 0)
						aniof(1, ord(g_touv[cx]));
				}

				if (g_caff == 13) {
					if (g_s._atticBallHoleObjectId == 141)
						aniof(1, 7);

					if (g_s._atticRodHoleObjectId == 159)
						aniof(1, 6);
				} else if ((g_caff == 14) && (g_s._cellarObjectId == 151))
					aniof(1, 2);
				else if ((g_caff == 17) && (g_s._secretPassageObjectId == 143))
					aniof(1, 1);
				else if ((g_caff == 24) && (g_s._wellObjectId != 0))
					aniof(1, 1);
			}
			
			if (g_caff < ROOM26)
				g_vm->startMusicOrSpeech(1);
		}
	}
}

void tinke() {
	Common::String d1 = g_vm->getEngineString(S_SHOULD_HAVE_NOTICED);
	Common::String d2 = g_vm->getEngineString(S_NUMBER_OF_HINTS);
	const char d3 = '[';
	const char d4 = ']';
	const char d5 = '1';
	Common::String d6 = g_vm->getEngineString(S_OK);
	int cf, day, hour, minute;
	Common::String stpo;

	g_vm->_anyone = false;
	g_vm->updateHour(day, hour, minute);
	if (day != g_vm->_day) {
		g_vm->_day = day;
		int i = 0;
		do {
			++i;
			if (g_nbrepm[i] != 0)
				--g_nbrepm[i];
			g_nbrep[i] = 0;
		} while (i != 8);
	}
	if ((hour > g_vm->_hour) || ((hour == 0) && (g_vm->_hour == 23))) {
		g_vm->_hour = hour;
		g_vm->_minute = 0;
		drawClock();
		cf = 0;
		for (int i = 1; i <= 10; ++i) {
			if (g_s._pourc[i] == '*')
				++cf;
		}

		if (cf == 10)
			stpo = "10";
		else
			stpo = chr(cf + 48);

		g_vm->_hintPctMessage = Common::String(d3);
		g_vm->_hintPctMessage += d5;
		g_vm->_hintPctMessage += d4;
		g_vm->_hintPctMessage += d3;
		g_vm->_hintPctMessage += d1;
		g_vm->_hintPctMessage += stpo;
		g_vm->_hintPctMessage += '0';
		g_vm->_hintPctMessage += d2;
		g_vm->_hintPctMessage += d4;
		g_vm->_hintPctMessage += d3;
		g_vm->_hintPctMessage += d6;
		g_vm->_hintPctMessage += d4;
	}
	if (minute > g_vm->_minute) {
		g_vm->_minute = 30;
		drawClock();
	}
	if (g_vm->_mouse.y_s < 12)
		return;

	if (!g_vm->_blo) {
		if ((hour == 12) || ((hour > 18) && (hour < 21)) || ((hour >= 0) && (hour < 7)))
			g_t = kTime2;
		else
			g_t = kTime1;
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
			switch (g_vm->_place) {
			case GREEN_ROOM:
			case DARKBLUE_ROOM:
				g_vm->setRandomPresenceGreenRoom(cf);
				break;
			case PURPLE_ROOM:
				g_vm->setRandomPresencePurpleRoom(cf);
				break;
			case BLUE_ROOM:
				g_vm->setRandomPresenceBlueRoom(cf);
				break;
			case RED_ROOM:
			case GREEN_ROOM2:
				g_vm->setRandomPresenceRedRoom(cf);
				break;
			case ROOM9:
				g_vm->setRandomPresenceRoom9(cf);
				break;
			case DINING_ROOM:
				g_vm->setRandomPresenceDiningRoom(cf);
				break;
			case BUREAU:
				g_vm->setRandomPresenceBureau(cf);
				break;
			case KITCHEN:
				g_vm->setRandomPresenceKitchen(cf);
				break;
			case ATTIC:
			case CELLAR:
				g_vm->setRandomPresenceAttic(cf);
				break;
			case LANDING:
			case ROOM26:
				g_vm->setRandomPresenceLanding(cf);
				break;
			case CHAPEL:
				g_vm->setRandomPresenceChapel(cf);
				break;
			}
			if ((g_mpers != 0) && (g_vm->_currBitIndex != 10))
				g_mpers = g_vm->_currBitIndex;

			if ((g_mpers == 0) && (g_vm->_currBitIndex > 0)) {
				if ((g_s._currPlace == ATTIC) || (g_s._currPlace == CELLAR)) {
					cavegre();
				} else if (g_vm->_currBitIndex == 10) {
					g_vm->_currBitIndex = 0;
					if (!g_vm->_brt) {
						g_vm->_brt = true;
						g_hdb = readclock();
						if (getRandomNumber(1, 5) < 5) {
							g_vm->clearScreenType3();
							g_vm->prepareScreenType2();
							ecr3(g_vm->getEngineString(S_HEAR_NOISE));
							int rand = (getRandomNumber(0, 4)) - 2;
							g_vm->_speechManager.startSpeech(1, rand, 1);
							g_vm->clearScreenType3();
						}
					}
				}
			}

			if (activeMenu)
				g_vm->_menu.drawMenu();
		}
	}
	g_hfb = readclock();
	if ((g_vm->_brt) && ((g_hfb - g_hdb) > 17)) {
		g_vm->getPresenceBitIndex(g_vm->_place);
		g_vm->_brt = false;
		g_hdb = 0;
		if ((g_s._currPlace > OWN_ROOM) && (g_s._currPlace < DINING_ROOM))
			g_vm->_anyone = true;
	}
}

void fenat(char ans) {
	int coul;

	g_vm->_mouse.hideMouse();
	if (g_vm->_currGraphicalDevice == MODE_CGA)
		coul = 2;
	else if (g_vm->_currGraphicalDevice == MODE_HERCULES)
		coul = 1;
	else
		coul = 12;

	g_vm->_screenSurface.writeCharacter(Common::Point(306, 193), ord(ans), coul);
	g_vm->_screenSurface.drawBox(300, 191, 16, 8, 15);
	g_vm->_mouse.showMouse();
}


/* NIVEAU 8 */
void afdes() {
	taffich();
	dessin(0);
	g_vm->_okdes = false;
}

void tkey1(bool d) {
	bool quest = false;
	int x, y, c;

	g_vm->_mouse.hideMouse();
	fenat('K');

	// Wait for release from any key or mouse button
	while (g_vm->keyPressed())
		g_key = testou();
	do {
		g_vm->_mouse.getMousePos_(x, y, c);
		g_vm->keyPressed();
	} while (c != 0);
	
	// Event loop
	do {
		if (d)
			tinke();
		quest = g_vm->keyPressed();
		g_vm->_mouse.getMousePos_(x, y, c);
		CHECK_QUIT;
	} while (!(quest || (c != 0) || (d && g_vm->_anyone)));
	if (quest)
		testou();
	g_vm->setMouseClick(false);
	g_vm->_mouse.showMouse();
}

/* NIVEAU 7 */
void tlu(int af, int ob) {
	g_caff = 32;
	afdes();
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

void modinv() {
	int r;
	Common::String nomp;

	int cy = 0;
	for (int i = 1; i <= 6; ++i) {
		if (g_s._sjer[i] != chr(0)) {
			++cy;
			r = (ord(g_s._sjer[i]) + 400);
			nomp = deline(r - 501 + kInventoryStringIndex);
			g_vm->_menu.setText(g_vm->_menu._inventoryMenu[cy], nomp);
			g_vm->_menu.enableMenuItem(g_vm->_menu._inventoryMenu[i]);
		}
	}

	if (cy < 6) {
		for (int i = cy + 1; i <= 6; ++i) {
			g_vm->_menu.setText(g_vm->_menu._inventoryMenu[i], "                       ");
			g_vm->_menu.disableMenuItem(g_vm->_menu._inventoryMenu[i]);
		}
	}
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

void tctrm() {
	repon(2, (3000 + g_ctrm));
	g_ctrm = 0;
}


void quelquun() {
	if (g_vm->_menu._menuDisplayed)
		g_vm->_menu.eraseMenu();

	g_vm->endSearch();
	g_crep = 997;
L1:
	if (!g_vm->_hiddenHero) {
		if (g_crep == 997)
			g_crep = 138;
		repon(2, g_crep);
		if (g_crep == 138)
			g_vm->_speechManager.startSpeech(5, 2, 1);
		else
			g_vm->_speechManager.startSpeech(4, 4, 1);

		if (g_iouv == 0)
			g_s._faithScore += 2;
		else if (g_s._faithScore < 50)
			g_s._faithScore += 4;
		else
			g_s._faithScore += 3 * (g_s._faithScore / 10);
		tsort();
		g_vm->_menu.setDestinationMenuText(LANDING);
		int cx = g_vm->convertBitIndexToCharacterIndex(g_vm->_currBitIndex);
		g_caff = 69 + cx;
		g_crep = g_caff;
		g_msg[3] = MENU_DISCUSS;
		g_msg[4] = g_vm->_menu._discussMenu[cx];
		g_vm->_syn = true;
		g_vm->_col = true;
	} else {
		if (getRandomNumber(1, 3) == 2) {
			g_vm->_hiddenHero = false;
			g_crep = 137;
			goto L1;
		} else {
			repon(2, 136);
			int rand = (getRandomNumber(0, 4)) - 2;
			g_vm->_speechManager.startSpeech(3, rand, 1);
			g_vm->clearScreenType2();
			g_vm->displayAloneText();
			debloc(21);
			affrep();
		}
	}
	if (g_vm->_menu._menuDisplayed)
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
		if (g_vm->_currBitIndex != 0)
			g_s._faithScore += 2;
	} else {
		affrep();
		g_vm->endSearch();
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
	bool inRect = false;
	do {
		touch = '\0';

		do {
			g_vm->_mouse.moveMouse(qust, touch);
			CHECK_QUIT;

			if (g_vm->getMouseClick())
				inRect = (g_vm->_mouse.x_s < 256 * g_res) && (g_vm->_mouse.y_s < 176) && (g_vm->_mouse.y_s > 12);
			tinke();
		} while (!(qust || inRect || g_vm->_anyone));

		if (qust && (touch == '\103'))
			Alert::show(g_vm->_hintPctMessage, 1);
	} while (!((touch == '\73') || ((touch == '\104') && (g_x != 0) && (g_y != 0)) || (g_vm->_anyone) || (inRect)));

	if (touch == '\73')
		g_vm->_keyPressedEsc = true;

	if (inRect) {
		g_x = g_vm->_mouse.x_s;
		g_y = g_vm->_mouse.y_s;
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
		afdes();
		if ((g_caff > 29) && (g_caff < 33))
			repon(2, g_caff);
		else
			repon(2, g_caff + 400);
		tkey1(true);
		g_caff = mdes;
		g_msg[3] = 0;
		g_crep = 998;
	} else {
		g_vm->_obpart = true;
		g_crep = g_caff + 400;
		g_vm->_menu.setSearchMenu();
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

int t23coul() {
	if (!g_vm->checkInventory(143)) {
		g_crep = 1512;
		g_vm->loseGame();
	}

	return CELLAR;
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
