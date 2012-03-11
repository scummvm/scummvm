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

/**
 * Engine function : Is mouse in a given rect?
 * @remarks	Originally called 'dans_rect'
 */
bool isMouseIn(rectangle r) {
	int x, y, c;

	getMousePos_(x, y, c);
	if ((x > r._x1) && (x < r._x2) && (y > r._y1) && (y < r._y2))
		return true;

	return false;
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
	hideMouse();
	s_sauv(g_vm->_currGraphicalDevice, y, dy);
	showMouse();
}

void charecr(int y, int dy) {
	hideMouse();
	s_char(g_vm->_currGraphicalDevice, y, dy);
	showMouse();
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
	hideMouse();
	writepal(g_numpal);
	pictout(ad, 0, x, y);
	showMouse();
}

void dessine_rouleau() {
	writepal(89);
	if (g_vm->_currGraphicalDevice == MODE_HERCULES) {
		g_mem[0x7000 * 16 + 14] = 15;
	}
	hideMouse();
	pictout(0x73a2, 0, 0, 0);
	showMouse();
}

/**
 * Set Text Color
 * @remarks	Originally called 'text_color'
 */
void setTextColor(int c) {
	g_vm->_textColor = c;
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

void ecrf1() {
	// Large drawing
	g_vm->_screenSurface.drawBox(0, 11, 512, 163, 15);
}

void clsf1() {
	hideMouse();
	g_vm->_screenSurface.fillRect(0, Common::Rect(0, 11, 514, 175));

	showMouse();
}

/**
 * Engine function - Clear Screen - Type 2
 * @remarks	Originally called 'clsf2'
 */
void clearScreenType2() {
	hideMouse();
	if (g_vm->_largestClearScreen) {
		g_vm->_screenSurface.fillRect(0, Common::Rect(1, 176, 633, 199));
		g_vm->_screenSurface.drawBox(0, 175, 634, 24, 15);
		g_vm->_largestClearScreen = false;
	} else {
		g_vm->_screenSurface.fillRect(0, Common::Rect(1, 176, 633, 190));
		g_vm->_screenSurface.drawBox(0, 175, 634, 15, 15);
	}
	showMouse();
}

void ecrf2() {
	setTextColor(5);
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
		clearScreenType2();
		g_vm->_screenSurface.putxy(8, 176);
		g_vm->_screenSurface.drawString(copy(str_, 1, (tlig - 1)), 5);
		g_vm->_screenSurface.putxy(8, 182);
		g_vm->_screenSurface.drawString(copy(str_, tlig, ((tlig << 1) - 1)), 5);
		g_vm->_screenSurface.putxy(8, 190);
		g_vm->_screenSurface.drawString(copy(str_, tlig << 1, tlig * 3), 5);
	}
}

void clsf3() {
	hideMouse();
	g_vm->_screenSurface.fillRect(0, Common::Rect(1, 192, 633, 199));
	g_vm->_screenSurface.drawBox(0, 191, 634, 8, 15);
	showMouse();
}

void ecr3(Common::String text) {
	clsf3();
	g_vm->_screenSurface.putxy(8, 192);
	g_vm->_screenSurface.drawString(text, 5);
}

void ecrf6() {
	setTextColor(5);
	g_vm->_screenSurface.drawBox(62, 33, 363, 80, 15);
}

void ecrf7() {
	setTextColor(4);
}

void clsf10() {
	int co, cod;
	Common::String st;

	hideMouse();
	if (g_res == 1) {
		co = 634;
		cod = 534;
	} else {
		co = 600;
		cod = 544;
	}
	g_vm->_screenSurface.fillRect(15, Common::Rect(cod, 93, co, 98));
	if (g_s._faithScore < 33)
		st = g_vm->getEngineString(S_COOL);
	else if (g_s._faithScore < 66)
		st = g_vm->getEngineString(S_LOURDE);
	else if (g_s._faithScore > 65)
		st = g_vm->getEngineString(S_MALSAINE);
	
	co = 580 - (g_vm->_screenSurface.getStringWidth(st) / 2);
	g_vm->_screenSurface.putxy(co, 92);
	g_vm->_screenSurface.drawString(st, 4);

	g_vm->_screenSurface.fillRect(15, Common::Rect(560, 24, 610, 86));
	/* rempli(69,12,32,5,255);*/
	showMouse();
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

/**
 * Engine function - Update hour
 * @remarks	Originally called 'calch'
 */
void updateHour(int &day, int &hour, int &minute) {
	int newHour = readclock();
	int th = g_jh + ((newHour - g_mh) / g_t);
	minute = ((th % 2) + g_vm__) * 30;
	hour = ((uint)th >> 1) + g_vh;
	if (minute == 60) {
		minute = 0;
		++hour;
	}
	day = (hour / 24) + g_vj;
	hour = hour - ((day - g_vj) * 24);
}

/**
 * Engine function - Convert character index to bit index
 * @remarks	Originally called 'conv'
 */
int convertCharacterIndexToBitIndex(int characterIndex) {
	return 128 >> (characterIndex - 1);
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
	Common::String str_;
	Common::String str1;
	Common::String tmpStr;

	if ((m > 499) && (m < 563)) {
		tmpStr = deline(m - 501 + kInventoryStringIndex);

		if ((int) tmpStr.size() > ((58 + (g_res - 1) * 37) << 1))
			g_vm->_largestClearScreen = true;
		else
			g_vm->_largestClearScreen = false;

		clearScreenType2();
		displayStr(tmpStr, 8, 176, 85, 3, 5);
	} else {
		modif(m);
		if (f == 8)
			f = 2;
		if (f == 1)
			f = 6;
		if (f == 2) {
			clearScreenType2();
			ecrf2();
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
		}
		if ((f == 6) || (f == 9)) {
			int i;
			if (f == 6)
				i = 4;
			else
				i = 5;

			tmpStr = deline(m);
			displayStr(tmpStr, 80, 40, 60, 25, i);

			if (m == 180)
				g_s._pourc[6] = '*';

			if (m == 179)
				g_s._pourc[10] = '*';
		}
		if (f == 7) {         /* messint */
			ecrf7();
			tmpStr = deline(m);

			int xSmallStr, xLargeStr, dx;
			if (g_res == 1) {
				xSmallStr = 252 - tmpStr.size() * 5;
				xLargeStr = 100;
				dx = 80;
			} else {
				xSmallStr = 252 - tmpStr.size() * 3;
				xLargeStr = 144;
				dx = 50;
			}

			if (tmpStr.size() < 40)
				displayStr(tmpStr, xSmallStr, 86, dx, 3, 5);
			else
				displayStr(tmpStr, xLargeStr, 86, dx, 3, 5);
		}
	}
}

void t5(int cx) {
	if (cx == 10)
		g_vm->_blo = false;

	if (cx != 1) {
		g_vm->_roomPresenceLuc = false;
		g_vm->_roomPresenceIda = false;
	}

	if (cx != 2)
		g_vm->_purpleRoomPresenceLeo = false;

	if (cx != 4) {
		g_vm->_roomPresenceGuy = false;
		g_vm->_roomPresenceEva = false;
	}

	if (cx != 5)
		g_vm->_roomPresenceMax = false;
	if (cx != 6)
		g_vm->_roomPresenceBob = false;
	if (cx != 8)
		g_vm->_roomPresencePat = false;
	if (cx != 3)
		g_vm->_toiletsPresenceBobMax = false;
	if (cx != 7)
		g_vm->_bathRoomPresenceBobMax = false;
	if (cx != 9)
		g_vm->_room9PresenceLeo = false;
}

/**
 * Engine function - Show the people present in the given room 
 * @remarks	Originally called 'affper'
 */
void showPeoplePresent(int per) {
	int cx;
	int xp = 580 - (g_vm->_screenSurface.getStringWidth("LEO") / 2);

	for (cx = 1; cx <= 8; ++cx)
		g_vm->_menu.disableMenuItem(g_vm->_menu._discussMenu[cx]);
	clsf10();
	if ((per & 128) == 128) {
		g_vm->_screenSurface.putxy(xp, 24);
		g_vm->_screenSurface.drawString("LEO", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._discussMenu[1]);
	}
	if ((per & 64) == 64) {
		g_vm->_screenSurface.putxy(xp, 32);
		g_vm->_screenSurface.drawString("PAT", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._discussMenu[2]);
	}
	if ((per & 32) == 32) {
		g_vm->_screenSurface.putxy(xp, 40);
		g_vm->_screenSurface.drawString("GUY", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._discussMenu[3]);
	}
	if ((per & 16) == 16) {
		g_vm->_screenSurface.putxy(xp, 48);
		g_vm->_screenSurface.drawString("EVA", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._discussMenu[4]);
	}
	if ((per & 8) == 8) {
		g_vm->_screenSurface.putxy(xp, 56);
		g_vm->_screenSurface.drawString("BOB", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._discussMenu[5]);
	}
	if ((per & 4) == 4) {
		g_vm->_screenSurface.putxy(xp, 64);
		g_vm->_screenSurface.drawString("LUC", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._discussMenu[6]);
	}
	if ((per & 2) == 2) {
		g_vm->_screenSurface.putxy(xp, 72);
		g_vm->_screenSurface.drawString("IDA", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._discussMenu[7]);
	}
	if ((per & 1) == 1) {
		g_vm->_screenSurface.putxy(xp, 80);
		g_vm->_screenSurface.drawString("MAX", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._discussMenu[8]);
	}
	g_ipers = per;
}

int selectCharacters(int min, int max) {
	bool invertSelection = false;
	int rand = getRandomNumber(min, max);

	if (rand > 4) {
		rand = 8 - rand;
		invertSelection = true;
	}

	int cx = 0;
	int retVal = 0;
	while (cx < rand) {
		int charIndex = getRandomNumber(1, 8);
		int charBitIndex = convertCharacterIndexToBitIndex(charIndex);
		if ((retVal & charBitIndex) != charBitIndex) {
			++cx;
			retVal |= charBitIndex;
		}
	}
	if (invertSelection)
		retVal = 255 - retVal;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Green Room
 * @remarks	Originally called 'cpl1'
 */
int getPresenceStatsGreenRoom() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	// The original uses an || instead of an &&, resulting 
	// in an always true condition. Based on the other tests, 
	// and on other scenes, we use an && instead.
	if ((hour > 7) && (hour < 11))
		retVal = 25;
	else if ((hour > 10) && (hour < 14))
		retVal = 35;
	else if ((hour > 13) && (hour < 16))
		retVal = 50;
	else if ((hour > 15) && (hour < 18))
		retVal = 5;
	else if ((hour > 17) && (hour < 22))
		retVal = 35;
	else if ((hour > 21) && (hour < 24))
		retVal = 50;
	else if ((hour >= 0) && (hour < 8))
		retVal = 70;

	g_vm->_menu.mdn();

	return retVal;
}
/**
 * Engine function - Get Presence Statistics - Purple Room
 * @remarks	Originally called 'cpl2'
 */
int getPresenceStatsPurpleRoom() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if ((hour > 7) && (hour < 11))
		retVal = -2;
	else if (hour == 11)
		retVal = 100;
	else if ((hour > 11) && (hour < 23))
		retVal = 10;
	else if (hour == 23)
		retVal = 20;
	else if ((hour >= 0) && (hour < 8))
		retVal = 50;
	
	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Toilets
 * @remarks	Originally called 'cpl3'
 */
int getPresenceStatsToilets() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if (((hour > 8) && (hour < 10)) || ((hour > 19) && (hour < 24)))
		retVal = 34;
	else if (((hour > 9) && (hour < 20)) || ((hour >= 0) && (hour < 9)))
		retVal = 0;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Blue Room
 * @remarks	Originally called 'cpl5'
 */
int getPresenceStatsBlueRoom() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if ((hour > 6) && (hour < 10))
		retVal = 0;
	else if (hour == 10)
		retVal = 100;
	else if ((hour > 10) && (hour < 24))
		retVal = 15;
	else if ((hour >= 0) && (hour < 7))
		retVal = 50;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Red Room
 * @remarks	Originally called 'cpl6'
 */
int getPresenceStatsRedRoom() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if (((hour > 7) && (hour < 13)) || ((hour > 17) && (hour < 20)))
		retVal = -2;
	else if (((hour > 12) && (hour < 17)) || ((hour > 19) && (hour < 24)))
		retVal = 35;
	else if (hour == 17)
		retVal = 100;
	else if ((hour >= 0) && (hour < 8))
		retVal = 60;

	return retVal;
}

/**
 * Shows the "you are alone" message in the status area
 * on the right hand side of the screen
 * @remarks	Originally called 'person'
 */
void displayAloneText() {
	for (int cf = 1; cf <= 8; ++cf)
		g_vm->_menu.disableMenuItem(g_vm->_menu._discussMenu[cf]);

	Common::String sYou = g_vm->getEngineString(S_YOU);
	Common::String sAre = g_vm->getEngineString(S_ARE);
	Common::String sAlone = g_vm->getEngineString(S_ALONE);

	clsf10();
	g_vm->_screenSurface.putxy(580 - (g_vm->_screenSurface.getStringWidth(sYou) / 2), 30);
	g_vm->_screenSurface.drawString(sYou, 4);
	g_vm->_screenSurface.putxy(580 - (g_vm->_screenSurface.getStringWidth(sAre) / 2), 50);
	g_vm->_screenSurface.drawString(sAre, 4);
	g_vm->_screenSurface.putxy(580 - (g_vm->_screenSurface.getStringWidth(sAlone) / 2), 70);
	g_vm->_screenSurface.drawString(sAlone, 4);

	g_ipers = 0;
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

	hideMouse();
	
	paint_rect(570, 118, 20, 10);
	paint_rect(578, 114, 6, 18);
	if ((g_vm->_currGraphicalDevice == MODE_CGA) || (g_vm->_currGraphicalDevice == MODE_HERCULES))
		co = 0;
	else
		co = 1;

	if (g_minute == 0)
		g_vm->_screenSurface.drawLine(((uint)x >> 1) * g_res, y, ((uint)x >> 1) * g_res, (y - rg), co);
	else 
		g_vm->_screenSurface.drawLine(((uint)x >> 1) * g_res, y, ((uint)x >> 1) * g_res, (y + rg), co);

	h = g_hour;
	if (h > 12)
		h -= 12;
	if (h == 0)
		h = 12;

	g_vm->_screenSurface.drawLine(((uint)x >> 1) * g_res, y, ((uint)(x + cv[0][h - 1]) >> 1) * g_res, y + cv[1][h - 1], co);
	showMouse();
	g_vm->_screenSurface.putxy(568, 154);

	if (g_hour > 11)
		g_vm->_screenSurface.drawString("PM ", 1);
	else
		g_vm->_screenSurface.drawString("AM ", 1);

	g_vm->_screenSurface.putxy(550, 160);
	if ((g_day >= 0) && (g_day <= 8)) {
		Common::String tmp = g_vm->getEngineString(S_DAY);
		tmp.insertChar((char)(g_day + 49), 0);
		g_vm->_screenSurface.drawString(tmp, 1);
	}
}

/*************
 * NIVEAU 11 *
 *************/

void debloc(int l) {
	g_num = 0;
	g_x = 0;
	g_y = 0;
	if ((l != 26) && (l != 15))
		t5(l);
	g_mpers = g_ipers;
}

/**
 * Engine function - Get Presence Statistics - Room Bureau
 * @remarks	Originally called 'cpl10'
 */
int getPresenceStatsDiningRoom(int &hour) {
	int day, minute;

	int retVal = 0;
	updateHour(day, hour, minute);
	if (((hour > 7) && (hour < 11)) || ((hour > 11) && (hour < 14)) || ((hour > 18) && (hour < 21)))
		retVal = 100;
	else if ((hour == 11) || ((hour > 20) && (hour < 24)))
		retVal = 45;
	else if (((hour > 13) && (hour < 17)) || (hour == 18))
		retVal = 35;
	else if (hour == 17)
		retVal = 60;
	else if ((hour >= 0) && (hour < 8))
		retVal = 5;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Room Bureau
 * @remarks	Originally called 'cpl11'
 */
int getPresenceStatsBureau(int &hour) {
	int day, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if (((hour > 8) && (hour < 12)) || ((hour > 20) && (hour < 24)))
		retVal = 25;
	else if (((hour > 11) && (hour < 14)) || ((hour > 18) && (hour < 21)))
		retVal = 5;
	else if ((hour > 13) && (hour < 17))
		retVal = 55;
	else if ((hour > 16) && (hour < 19))
		retVal = 45;
	else if ((hour >= 0) && (hour < 9))
		retVal = 0;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Room Kitchen
 * @remarks	Originally called 'cpl12'
 */
int getPresenceStatsKitchen() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if (((hour > 8) && (hour < 15)) || ((hour > 16) && (hour < 22)))
		retVal = 55;
	else if (((hour > 14) && (hour < 17)) || ((hour > 21) && (hour < 24)))
		retVal = 25;
	else if ((hour >= 0) && (hour < 5))
		retVal = 0;
	else if ((hour > 4) && (hour < 9))
		retVal = 15;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Room Attic
 * @remarks	Originally called 'cpl13'
 */
int getPresenceStatsAttic() {
	return 0;
}

/**
 * Engine function - Get Presence Statistics - Room Landing
 * @remarks	Originally called 'cpl15'
 */
int getPresenceStatsLanding() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if ((hour > 7) && (hour < 12))
		retVal = 25;
	else if ((hour > 11) && (hour < 14))
		retVal = 0;
	else if ((hour > 13) && (hour < 18))
		retVal = 10;
	else if ((hour > 17) && (hour < 20))
		retVal = 55;
	else if ((hour > 19) && (hour < 22))
		retVal = 5;
	else if ((hour > 21) && (hour < 24))
		retVal = 15;
	else if ((hour >= 0) && (hour < 8))
		retVal = -15;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Room Chapel
 * @remarks	Originally called 'cpl20'
 */
int getPresenceStatsChapel(int &hour) {
	int day, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if (hour == 10)
		retVal = 65;
	else if ((hour > 10) && (hour < 21))
		retVal = 5;
	else if ((hour > 20) && (hour < 24))
		retVal = -15;
	else if ((hour >= 0) && (hour < 5))
		retVal = -300;
	else if ((hour > 4) && (hour < 10))
		retVal = -5;

	return retVal;
}

/**
 * Engine function - Check who is in the Green Room
 * @remarks	Originally called 'quelq1'
 */
void setPresenceGreenRoom(int roomId) {
	int rand = getRandomNumber(1, 2);
	if (roomId == GREEN_ROOM) {
		if (rand == 1)
			g_vm->_roomPresenceLuc = true;
		else
			g_vm->_roomPresenceIda = true;
	} else if (roomId == DARKBLUE_ROOM) {
		if (rand == 1)
			g_vm->_roomPresenceGuy = true;
		else
			g_vm->_roomPresenceEva = true;
	}

	g_ipers = 10;
}

/**
 * Engine function - Check who is in the Purple Room
 * @remarks	Originally called 'quelq2'
 */
void setPresencePurpleRoom() {
	if (g_li == 2)
		g_vm->_purpleRoomPresenceLeo = true;
	else
		g_vm->_room9PresenceLeo = true;

	g_ipers = 10;
}

/**
 * Engine function - Check who is in the Blue Room
 * @remarks	Originally called 'quelq5'
 */
void setPresenceBlueRoom() {
	g_vm->_roomPresenceMax = true;
	g_ipers = 10;
}

/**
 * Engine function - Check who is in the Red Room
 * @remarks	Originally called 'quelq6'
 */
void setPresenceRedRoom(int l) {
	if (l == 6)
		g_vm->_roomPresenceBob = true;
	else if (l == 8)
		g_vm->_roomPresencePat = true;

	g_ipers = 10;
}

/**
 * Engine function - Check who is in the Dining Room
 * @remarks	Originally called 'quelq10'
 */
int setPresenceDiningRoom(int hour) {
	int retVal = 0;

	if ((hour >= 0) && (hour < 8))
		retVal = chlm();
	else {
		int min = 0, max = 0;
		if ((hour > 7) && (hour < 10)) {
			min = 5;
			max = 7;
		} else if ((hour > 9) && (hour < 12)) {
			min = 1;
			max = 4;
		} else if (((hour > 11) && (hour < 15)) || ((hour > 18) && (hour < 21))) {
			min = 6;
			max = 8;
		} else if (((hour > 14) && (hour < 19)) || ((hour > 20) && (hour < 24))) {
			min = 1;
			max = 5;
		}
		retVal = selectCharacters(min, max);
	}
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Check who is in the Bureau
 * @remarks	Originally called 'quelq11'
 */
int setPresenceBureau(int hour) {
	int retVal = 0;

	if ((hour >= 0) && (hour < 8))
		retVal = chlm();
	else {
		int min = 0, max = 0;
		if (((hour > 7) && (hour < 10)) || ((hour > 20) && (hour < 24))) {
			min = 1;
			max = 3;
		} else if (((hour > 9) && (hour < 12)) || ((hour > 13) && (hour < 19))) {
			min = 1;
			max = 4;
		} else if (((hour > 11) && (hour < 14)) || ((hour > 18) && (hour < 21))) {
			min = 1;
			max = 2;
		}
		retVal = selectCharacters(min, max);
	}
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Check who is in the Kitchen
 * @remarks	Originally called 'quelq12'
 */
int setPresenceKitchen() {
	int retVal = chlm();
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Check who is in the Landing
 * @remarks	Originally called 'quelq15'
 */
int setPresenceLanding() {
	bool test = false;
	int rand = 0;
	do {
		rand = getRandomNumber(1, 8);
		test = (((rand == 1) && (g_vm->_purpleRoomPresenceLeo || g_vm->_room9PresenceLeo)) ||
		        ((rand == 2) && g_vm->_roomPresencePat) ||
		        ((rand == 3) && g_vm->_roomPresenceGuy) ||
		        ((rand == 4) && g_vm->_roomPresenceEva) ||
		        ((rand == 5) && g_vm->_roomPresenceBob) ||
		        ((rand == 6) && g_vm->_roomPresenceLuc) ||
		        ((rand == 7) && g_vm->_roomPresenceIda) ||
		        ((rand == 8) && g_vm->_roomPresenceMax));
	} while (test);

	int retVal = convertCharacterIndexToBitIndex(rand);
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Check who is in the chapel
 * @remarks	Originally called 'quelq20'
 */
int setPresenceChapel(int hour) {
	int retVal = 0;

	if (((hour >= 0) && (hour < 10)) || ((hour > 18) && (hour < 24)))
		retVal = chlm();
	else {
		int min = 0, max = 0;
		if ((hour > 9) && (hour < 12)) {
			min = 3;
			max = 7;
		} else if ((hour > 11) && (hour < 18)) {
			min = 1;
			max = 2;
		} else if (hour == 18) {
			min = 2;
			max = 4;
		}
		retVal = selectCharacters(min, max);
	}
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Get the answer after you known a door
 * @remarks	Originally called 'frap'
 */
void getKnockAnswer() {
	int day, hour, minute;

	updateHour(day, hour, minute);
	if ((hour >= 0) && (hour < 8))
		g_crep = 190;
	else {
		if (getRandomNumber(1, 100) > 70)
			g_crep = 190;
		else
			g_crep = 147;
	}
}

int nouvp(int l) {
	int bitIndex = 0;
	if (l == 1) {
		if (g_vm->_roomPresenceLuc)
			bitIndex = 4;  // LUC
		if (g_vm->_roomPresenceIda)
			bitIndex = 2;  // IDA
	} else if (((l == 2) && (g_vm->_purpleRoomPresenceLeo)) || ((l == 9) && (g_vm->_room9PresenceLeo)))
		bitIndex = 128;    // LEO
	else if (l == 4) {
		if (g_vm->_roomPresenceGuy)
			bitIndex = 32; // GUY
		if (g_vm->_roomPresenceEva)
			bitIndex = 16; // EVA
	} else if ((l == 5) && (g_vm->_roomPresenceMax))
		bitIndex = 1;      // MAX
	else if ((l == 6) && (g_vm->_roomPresenceBob))
		bitIndex = 8;      // BOB
	else if ((l == 8) && (g_vm->_roomPresencePat))
		bitIndex = 64;     // PAT
	else if (((l == 3) && (g_vm->_toiletsPresenceBobMax)) || ((l == 7) && (g_vm->_bathRoomPresenceBobMax)))
		bitIndex = 9;      // BOB + MAX

	if (bitIndex != 9)
		showPeoplePresent(bitIndex);

	return bitIndex;
}

/**
 * Engine function - Convert bit index to character index
 * @remarks	Originally called 'tip'
 */
int convertBitIndexToCharacterIndex(int bitIndex) {
	int retVal = 0;

	if (bitIndex == 128)
		retVal = 1;
	else if (bitIndex == 64)
		retVal = 2;
	else if (bitIndex == 32)
		retVal = 3;
	else if (bitIndex == 16)
		retVal = 4;
	else if (bitIndex == 8)
		retVal = 5;
	else if (bitIndex == 4)
		retVal = 6;
	else if (bitIndex == 2)
		retVal = 7;
	else if (bitIndex == 1)
		retVal = 8;

	return retVal;
}


void ecfren(int &p, int &rand, int cf, int l) {
	if (l == 0)
		displayAloneText();
	p = -500;
	rand = 0;
	if (((l == 1) && (!g_vm->_roomPresenceLuc) && (!g_vm->_roomPresenceIda)) || ((l == 4) && (!g_vm->_roomPresenceGuy) && (!g_vm->_roomPresenceEva)))
		p = getPresenceStatsGreenRoom();
	if ((l == 2) && (!g_vm->_purpleRoomPresenceLeo) && (!g_vm->_room9PresenceLeo))
		p = getPresenceStatsPurpleRoom();
	if (((l == 3) && (!g_vm->_toiletsPresenceBobMax)) || ((l == 7) && (!g_vm->_bathRoomPresenceBobMax)))
		p = getPresenceStatsToilets();
	if ((l == 5) && (!g_vm->_roomPresenceMax))
		p = getPresenceStatsBlueRoom();
	if (((l == 6) && (!g_vm->_roomPresenceBob)) || ((l == 8) && (!g_vm->_roomPresencePat)))
		p = getPresenceStatsRedRoom();
	if ((l == 9) && (!g_vm->_room9PresenceLeo) && (!g_vm->_purpleRoomPresenceLeo))
		p = 10;
	if (((l == 2) && (g_vm->_room9PresenceLeo)) || ((l == 9) && (g_vm->_purpleRoomPresenceLeo)))
		p = -400;
	if (p != -500) {
		p += cf;
		rand = getRandomNumber(1, 100);
	}
}

void becfren(int l) {
	if ((l == 1) || (l == 4)) {
		int rand = getRandomNumber(1, 2);
		if (l == 1) {
			if (rand == 1)
				g_vm->_roomPresenceLuc = true;
			else
				g_vm->_roomPresenceIda = true;
		} else { // l == 4
			if (rand == 1)
				g_vm->_roomPresenceGuy = true;
			else
				g_vm->_roomPresenceEva = true;
		}
	} else if (l == 2)
		g_vm->_purpleRoomPresenceLeo = true;
	else if (l == 3)
		g_vm->_toiletsPresenceBobMax = true;
	else if (l == 5)
		g_vm->_roomPresenceMax = true;
	else if (l == 6)
		g_vm->_roomPresenceBob = true;
	else if (l == 7)
		g_vm->_bathRoomPresenceBobMax = true;
	else if (l == 8)
		g_vm->_roomPresencePat = true;
	else if (l == 9)
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

	g_s._ipre  = false;
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

	for (int cx = 2; cx <= 6; ++cx)
		g_s._sjer[cx] = chr(0);

	g_s._sjer[1] = chr(113);
	g_s._heure = chr(20);

	for (int cx = 1; cx <= 10; ++cx)
		g_s._pourc[cx] = ' ';

	for (int cx = 1; cx <= 6; ++cx)
		g_s._teauto[cx] = '*';

	for (int cx = 7; cx <= 9; ++cx)
		g_s._teauto[cx] = ' ';

	for (int cx = 10; cx <= 28; ++cx)
		g_s._teauto[cx] = '*';

	for (int cx = 29; cx <= 42; ++cx)
		g_s._teauto[cx] = ' ';

	g_s._teauto[33] = '*';

	for (int cx = 1; cx <= 8; ++cx)
		g_nbrep[cx] = 0;

	init_nbrepm();
}

void dprog() {
	g_li = 21;
	g_jh = 0;
	if (!g_s._ipre)
		g_vm->_blo = true;
	g_t = kTime1;
	g_mh = readclock();
}

void pl1(int cf) {
	if (((g_li == 1) && (!g_vm->_roomPresenceLuc) && (!g_vm->_roomPresenceIda)) || ((g_li == 4) && (!g_vm->_roomPresenceGuy) && (!g_vm->_roomPresenceEva))) {
		int p = getPresenceStatsGreenRoom();
		int rand;
		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresenceGreenRoom(g_li);
	}
}

void pl2(int cf) {
	if (!g_vm->_purpleRoomPresenceLeo) {
		int p = getPresenceStatsPurpleRoom();
		int rand;
		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresencePurpleRoom();
	}
}

void pl5(int cf) {
	if (!g_vm->_roomPresenceMax) {
		int p = getPresenceStatsBlueRoom();
		int rand;

		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresenceBlueRoom();
	}
}

void pl6(int cf) {
	if (((g_li == 6) && (!g_vm->_roomPresenceBob)) || ((g_li == 8) && (!g_vm->_roomPresencePat))) {
		int p = getPresenceStatsRedRoom();
		int rand;

		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresenceRedRoom(g_li);
	}
}

void pl9(int cf) {
	if (!g_vm->_room9PresenceLeo) {
		cf = -10;
		int p, rand;
		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresencePurpleRoom();
	}
}

void pl10(int cf) {
	int h, rand;
	int p = getPresenceStatsDiningRoom(h);
	phaz(rand, p, cf);

	if (rand > p)
		displayAloneText();
	else
		setPresenceDiningRoom(h);
}

void pl11(int cf) {
	int h, rand;

	int p = getPresenceStatsBureau(h);
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceBureau(h);
}

void pl12(int cf) {
	int p, rand;

	p = getPresenceStatsKitchen();
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceKitchen();
}

void pl13(int cf) {
	int p, rand;

	p = getPresenceStatsAttic();
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceKitchen();
}

void pl15(int cf) {
	int p, rand;

	p = getPresenceStatsLanding();
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceLanding();
}

void pl20(int cf) {
	int h, rand;

	int p = getPresenceStatsChapel(h);
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceChapel(h);
}

int t11(int l11) {
	int retVal = 0;
	int p, rand;

	ecfren(p, rand, g_s._faithScore, l11);
	g_li = l11;
	if ((l11 > 0) && (l11 < 10)) {
		if (p != -500) {
			if (rand > p) {
				displayAloneText();
				retVal = 0;
			} else {
				becfren(g_li);
				retVal = nouvp(g_li);
			}
		} else
			retVal = nouvp(g_li);
	}

	if (l11 > 9) {
		if ((l11 > 15) && (l11 != 20) && (l11 != 26))
			displayAloneText();
		else {
			int h = 0;
			if (l11 == 10)
				p = getPresenceStatsDiningRoom(h);
			else if (l11 == 11)
				p = getPresenceStatsBureau(h);
			else if (l11 == 12)
				p = getPresenceStatsKitchen();
			else if ((l11 == 13) || (l11 == 14))
				p = getPresenceStatsAttic();
			else if ((l11 == 15) || (l11 == 26))
				p = getPresenceStatsLanding();
			else if (l11 == 20)
				p = getPresenceStatsChapel(h);
			p += g_s._faithScore;
			rand = getRandomNumber(1, 100);
			if (rand > p) {
				displayAloneText();
				retVal = 0;
			} else {
				if (l11 == 10)
					p = setPresenceDiningRoom(h);
				else if (l11 == 11)
					p = setPresenceBureau(h);
				else if ((l11 == 12) || (l11 == 13) || (l11 == 14))
					p = setPresenceKitchen();
				else if ((l11 == 15) || (l11 == 26))
					p = setPresenceLanding();
				else if (l11 == 20)
					p = setPresenceChapel(h);
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
	clsf3();
	ecrf2();
	ecr3(g_vm->getEngineString(S_SOMEONE_ENTERS));
	int rand = (getRandomNumber(0, 4)) - 2;
	startSpeech(2, rand, 1);

	// The original was doing here a useless loop.
	// It has been removed

	clsf3();
	displayAloneText();
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

	ecrf1();
}

void musique(int so) {
	if (so == 0) {
		/* musik(0) */
		;
	} else if ((g_prebru == 0) && (!g_s._ipre)) {
		startSpeech(10, 1, 1);
		++g_prebru;
	} else {
		bool i = false;
		if ((g_s._currPlace == MOUNTAIN) || (g_s._currPlace == MANOR_FRONT) || (g_s._currPlace == MANOR_BACK)) {
			if (getRandomNumber(1, 3) == 2) {
				startSpeech(9, getRandomNumber(2, 4), 1);
				i = true;
			}
		}
		else if (g_s._currPlace == CHAPEL) {
			if (getRandomNumber(1, 2) == 1) {
				startSpeech(8, 1, 1);
				i = true;
			}
		}
		else if (g_s._currPlace == WELL) {
			if (getRandomNumber(1, 2) == 2) {
				startSpeech(12, 1, 1);
				i = true;
			}
		}
		else if (g_s._currPlace == 23) {
			startSpeech(13, 1, 1);
			i = true;
		}

		if (!i)
			startSpeech(getRandomNumber(1, 17), 1, 2);
	}
}

/* NIVEAU 9 */
void dessin(int ad) {
	if (ad != 0)
		dessine(g_ades, ((ad % 160) * 2), (ad / 160));
	else {
		clsf1();
		if (g_caff > 99) {
			dessine(g_ades, 60, 33);
			g_vm->_screenSurface.drawBox(118, 32, 291, 121, 15);         // Medium box
		} else if (g_caff > 69) {
			dessine(g_ades, 112, 48);           // Heads
			g_vm->_screenSurface.drawBox(222, 47, 155, 91, 15);
		} else {
			dessine(g_ades, 0, 12);
			ecrf1();
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
			
			if (g_caff < 26)
				musique(1);
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
	int cx, cf, day, hour, minute;
	Common::String stpo;

	g_vm->_anyone = false;
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
					if (!g_vm->_brt) {
						g_vm->_brt = true;
						g_hdb = readclock();
						if (getRandomNumber(1, 5) < 5) {
							clsf3();
							ecrf2();
							ecr3(g_vm->getEngineString(S_HEAR_NOISE));
							int rand = (getRandomNumber(0, 4)) - 2;
							startSpeech(1, rand, 1);
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
	if ((g_vm->_brt) && ((g_hfb - g_hdb) > 17)) {
		cx = nouvp(g_li);
		g_vm->_brt = false;
		g_hdb = 0;
		if ((g_s._currPlace > OWN_ROOM) && (g_s._currPlace < DINING_ROOM))
			g_vm->_anyone = true;
	}
}

void fenat(char ans) {
	int coul;

	hideMouse();
	if (g_vm->_currGraphicalDevice == MODE_CGA)
		coul = 2;
	else if (g_vm->_currGraphicalDevice == MODE_HERCULES)
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
	g_vm->_okdes = false;
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
		getMousePos_(x, y, c);
		keypressed();
	} while (c != 0);
	
	// Event loop
	do {
		if (d)
			tinke();
		quest = keypressed();
		getMousePos_(x, y, c);
		CHECK_QUIT;
	} while (!(quest || (c != 0) || (d && g_vm->_anyone)));
	if (quest)
		testou();
	g_vm->setMouseClick(false);
	showMouse();
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

/* NIVEAU 6 */

void tperd() {
	initouv();
	g_ment = 0;
	g_iouv = 0;
	g_mchai = 0;
	g_vm->_menu.unsetSearchMenu();
	if (!g_vm->_blo)
		t11(21);

	g_vm->_loseGame = true;
	clsf1();
	g_vm->_screenSurface.drawBox(60, 35, 400, 50, 15);
	repon(9, g_crep);
	clearScreenType2();
	clsf3();
	g_vm->_col = false;
	g_vm->_syn = false;
	g_vm->_okdes = false;
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
			g_vm->_menu.setText(g_vm->_menu._inventoryMenu[cy], nomp);
			g_vm->_menu.enableMenuItem(g_vm->_menu._inventoryMenu[cx]);
		}
	if (cy < 6)
		for (int cx = cy + 1; cx <= 6; ++cx) {
			g_vm->_menu.setText(g_vm->_menu._inventoryMenu[cx], "                       ");
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
		startSpeech(repint, haut[g_caff - 69], 0);
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
	g_vm->_heroSearching = false;
	g_vm->_obpart = false;
	g_cs = 0;
	g_is = 0;
	g_vm->_menu.unsetSearchMenu();
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
		if (!g_vm->_blo)
			minute = t11(0);
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
		g_vm->_okdes = true;
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
	if (g_vm->_menu._menuDisplayed)
		g_vm->_menu.eraseMenu();

	endSearch();
	g_crep = 997;
L1:
	if (!g_vm->_hiddenHero) {
		if (g_crep == 997)
			g_crep = 138;
		repon(2, g_crep);
		if (g_crep == 138)
			startSpeech(5, 2, 1);
		else
			startSpeech(4, 4, 1);

		if (g_iouv == 0)
			g_s._faithScore += 2;
		else if (g_s._faithScore < 50)
			g_s._faithScore += 4;
		else
			g_s._faithScore += 3 * (g_s._faithScore / 10);
		tsort();
		g_vm->_menu.setDestinationMenuText(LANDING);
		int cx = convertBitIndexToCharacterIndex(g_ipers);
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
			startSpeech(3, rand, 1);
			clearScreenType2();
			displayAloneText();
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
	bool inRect = false;
	do {
		touch = '\0';

		do {
			moveMouse(qust, touch);
			CHECK_QUIT;

			if (g_vm->getMouseClick())
				inRect = (x_s < 256 * g_res) && (y_s < 176) && (y_s > 12);
			tinke();
		} while (!(qust || inRect || g_vm->_anyone));

		if (qust && (touch == '\103'))
			Alert::show(g_hintPctMessage, 1);
	} while (!((touch == '\73') || ((touch == '\104') && (g_x != 0) && (g_y != 0)) || (g_vm->_anyone) || (inRect)));

	if (touch == '\73')
		g_vm->_keyPressedEsc = true;

	if (inRect) {
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

/**
 * Engine function - Change Graphical Device
 * @remarks	Originally called 'change_gd'
 */
void changeGraphicalDevice(int newDevice) {
	hideMouse();
	g_vm->_currGraphicalDevice = newDevice;
	hirs();
	initMouse();
	showMouse();
	dessine_rouleau();
	tinke();
	drawClock();
	if (g_ipers != 0)
		showPeoplePresent(g_ipers);
	else
		displayAloneText();
	clearScreenType2();
	clsf3();
	g_maff = 68;
	afdes(0);
	repon(2, g_crep);
	g_vm->_menu.displayMenu();
}

/**
 * Called when a savegame has been loaded.
 * @remarks	Originally called 'antegame'
 */
void MortevielleEngine::gameLoaded() {
	hideMouse();
	_menu._menuDisplayed = false;
	_loseGame = true;
	_anyone = false;
	_okdes = true;
	_col = false;
	_hiddenHero = false;
	_brt = false;
	g_maff = 68;
	g_mnumo = 0;
	g_prebru = 0;
	g_x = 0;
	g_y = 0;
	g_num = 0;
	g_hdb = 0;
	g_hfb = 0;
	g_cs = 0;
	g_is = 0;
	g_ment = 0;
	_syn = true;
	_heroSearching = true;
	g_mchai = 0;
	g_inei = 0;
	initouv();
	g_iouv = 0;
	g_dobj = 0;
	affrep();
	g_hintPctMessage = deline(580);

	_okdes = false;
	_endGame = true;
	_loseGame = false;
	_heroSearching = false;

	displayAloneText();
	tinke();
	drawClock();
	afdes(0);
	repon(2, g_crep);
	clsf3();
	_endGame = false;
	g_vm->_menu.setDestinationMenuText(g_s._currPlace);
	modinv();
	if (g_s._selectedObjectId != 0)
		modobj(g_s._selectedObjectId + 400);
	showMouse();
}


/* NIVEAU 3 */
/* procedure PROGRAMME */
void tmaj3() {
	int day, hour, minute;

	updateHour(day, hour, minute);
	if (minute == 30)
		minute = 1;
	hour += day * 24;
	minute += hour * 2;
	g_s._heure = chr(minute);
}

/**
 * Engine function - Handle OpCodes
 * @remarks	Originally called 'tsitu'
 */
void MortevielleEngine::handleOpcode() {
	if (!_col)
		clearScreenType2();
	_syn = false;
	_keyPressedEsc = false;
	if (!_anyone) {
		if (_brt) {
			if ((g_msg[3] == MENU_MOVE) || (g_msg[4] == OPCODE_LEAVE) || (g_msg[4] == OPCODE_SLEEP) || (g_msg[4] == OPCODE_EAT)) {
				g_ctrm = 4;
				mennor();
				return;
			}
		}
		if (g_msg[3] == MENU_MOVE)
			fctMove();
		if (g_msg[3] == MENU_DISCUSS)
			fctDiscuss();
		if (g_msg[3] == MENU_INVENTORY)
			fctInventoryTake();
		if (g_msg[4] == OPCODE_ATTACH)
			fctAttach();
		if (g_msg[4] == OPCODE_WAIT)
			fctWait();
		if (g_msg[4] == OPCODE_FORCE)
			fctForce();
		if (g_msg[4] == OPCODE_SLEEP)
			fctSleep();
		if (g_msg[4] == OPCODE_LISTEN)
			fctListen();
		if (g_msg[4] == OPCODE_ENTER)
			fctEnter();
		if (g_msg[4] == OPCODE_CLOSE)
			fctClose();
		if (g_msg[4] == OPCODE_SEARCH)
			fctSearch();
		if (g_msg[4] == OPCODE_KNOCK)
			fctKnock();
		if (g_msg[4] == OPCODE_SCRATCH)
			fctScratch();
		if (g_msg[4] == OPCODE_READ)
			fctRead();
		if (g_msg[4] == OPCODE_EAT)
			fctEat();
		if (g_msg[4] == OPCODE_PLACE)
			fctPlace();
		if (g_msg[4] == OPCODE_OPEN)
			fctOpen();
		if (g_msg[4] == OPCODE_TAKE)
			fctTake();
		if (g_msg[4] == OPCODE_LOOK)
			fctLook();
		if (g_msg[4] == OPCODE_SMELL)
			fctSmell();
		if (g_msg[4] == OPCODE_SOUND)
			fctSound();
		if (g_msg[4] == OPCODE_LEAVE)
			fctLeave();
		if (g_msg[4] == OPCODE_LIFT)
			fctLift();
		if (g_msg[4] == OPCODE_TURN)
			fctTurn();
		if (g_msg[4] == OPCODE_SSEARCH)
			fctSelfSearch();
		if (g_msg[4] == OPCODE_SREAD)
			fctSelfRead();
		if (g_msg[4] == OPCODE_SPUT)
			fctSelfPut();
		if (g_msg[4] == OPCODE_SLOOK)
			fctSelftLook();
		_hiddenHero = false;

		if (g_msg[4] == OPCODE_SHIDE)
			fctSelfHide();
	} else {
		if (_anyone) {
			quelquun();
			_anyone = false;
			mennor();
			return;
		}
	}
	int hour, day, minute;
	updateHour(day, hour, minute);
	if ((((hour == 12) || (hour == 13) || (hour == 19)) && (g_s._currPlace != 10)) ||
	        ((hour > 0) && (hour < 6) && (g_s._currPlace != 0)))
		++g_s._faithScore;
	if (((g_s._currPlace < CRYPT) || (g_s._currPlace > MOUNTAIN)) && (g_s._currPlace != 23)
	        && (g_s._currPlace != 0) && (g_s._selectedObjectId != 152) && (!_loseGame)) {
		if ((g_s._faithScore > 99) && (hour > 8) && (hour < 16)) {
			g_crep = 1501;
			tperd();
		}
		if ((g_s._faithScore > 99) && (hour > 0) && (hour < 9)) {
			g_crep = 1508;
			tperd();
		}
		if ((day > 1) && (hour > 8) && (!_loseGame)) {
			g_crep = 1502;
			tperd();
		}
	}
	mennor();
}

/* NIVEAU 1 */

void theure() {
	g_vj = ord(g_s._heure);
	g_vh = g_vj % 48;
	g_vj /= 48;
	g_vm__ = g_vh % 2;
	g_vh /= 2;
	g_hour = g_vh;
	if (g_vm__ == 1)
		g_minute = 30;
	else
		g_minute = 0;
}
} // End of namespace Mortevielle
