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

/**
 * Draw right frame
 * @remarks	Originally called 'dessine_rouleau'
 */
void drawRightFrame() {
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
void MortevielleEngine::setTextColor(int col) {
	_textColor = col;
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

/**
 * Engine function - Clear Screen - Type 1
 * @remarks	Originally called 'clsf1'
 */
void clearScreenType1() {
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

/**
 * Engine function - Clear Screen - Type 3
 * @remarks	Originally called 'clsf3'
 */
void clearScreenType3() {
	hideMouse();
	g_vm->_screenSurface.fillRect(0, Common::Rect(1, 192, 633, 199));
	g_vm->_screenSurface.drawBox(0, 191, 634, 8, 15);
	showMouse();
}

/**
 * Engine function - Clear Screen - Type 10
 * @remarks	Originally called 'clsf10'
 */
void clearScreenType10() {
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

void ecr3(Common::String text) {
	clearScreenType3();
	g_vm->_screenSurface.putxy(8, 192);
	g_vm->_screenSurface.drawString(text, 5);
}

/**
 * Prepare screen - Type 1!
 * @remarks	Originally called 'ecrf1'
 */
void MortevielleEngine::prepareScreenType1() {
	// Large drawing
	_screenSurface.drawBox(0, 11, 512, 163, 15);
}

/**
 * Prepare room - Type 2!
 * @remarks	Originally called 'ecrf2'
 */
void MortevielleEngine::prepareScreenType2() {
	setTextColor(5);
}

/**
 * Prepare room - Type 3!
 * @remarks	Originally called 'ecrf7'
 */
void MortevielleEngine::prepareScreenType3() {
	setTextColor(4);
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
void MortevielleEngine::updateHour(int &day, int &hour, int &minute) {
	int newHour = readclock();
	int th = g_jh + ((newHour - g_mh) / g_t);
	minute = ((th % 2) + _currHalfHour) * 30;
	hour = ((uint)th >> 1) + _currHour;
	if (minute == 60) {
		minute = 0;
		++hour;
	}
	day = (hour / 24) + _currDay;
	hour = hour - ((day - _currDay) * 24);
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
	if ((m > 499) && (m < 563)) {
		Common::String tmpStr = deline(m - 501 + kInventoryStringIndex);

		if ((int) tmpStr.size() > ((58 + (g_res - 1) * 37) << 1))
			g_vm->_largestClearScreen = true;
		else
			g_vm->_largestClearScreen = false;

		clearScreenType2();
		displayStr(tmpStr, 8, 176, 85, 3, 5);
	} else {
		modif(m);
		switch (f) {
		case 2:
		case 8:
			clearScreenType2();
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

/**
 * Engine function - Reset presence in other rooms
 * @remarks	Originally called 't5'
 */
void MortevielleEngine::resetPresenceInRooms(int roomId) {
	if (roomId == DINING_ROOM)
		_blo = false;

	if (roomId != GREEN_ROOM) {
		_roomPresenceLuc = false;
		_roomPresenceIda = false;
	}

	if (roomId != PURPLE_ROOM)
		_purpleRoomPresenceLeo = false;

	if (roomId != DARKBLUE_ROOM) {
		_roomPresenceGuy = false;
		_roomPresenceEva = false;
	}

	if (roomId != BLUE_ROOM)
		_roomPresenceMax = false;
	if (roomId != RED_ROOM)
		_roomPresenceBob = false;
	if (roomId != GREEN_ROOM2)
		_roomPresencePat = false;
	if (roomId != TOILETS)
		_toiletsPresenceBobMax = false;
	if (roomId != BATHROOM)
		_bathRoomPresenceBobMax = false;
	if (roomId != ROOM9)
		_room9PresenceLeo = false;
}

/**
 * Engine function - Show the people present in the given room 
 * @remarks	Originally called 'affper'
 */
void MortevielleEngine::showPeoplePresent(int bitIndex) {
	int xp = 580 - (_screenSurface.getStringWidth("LEO") / 2);

	for (int i = 1; i <= 8; ++i)
		_menu.disableMenuItem(_menu._discussMenu[i]);

	clearScreenType10();
	if ((bitIndex & 128) == 128) {
		_screenSurface.putxy(xp, 24);
		_screenSurface.drawString("LEO", 4);
		_menu.enableMenuItem(_menu._discussMenu[1]);
	}
	if ((bitIndex & 64) == 64) {
		_screenSurface.putxy(xp, 32);
		_screenSurface.drawString("PAT", 4);
		_menu.enableMenuItem(_menu._discussMenu[2]);
	}
	if ((bitIndex & 32) == 32) {
		_screenSurface.putxy(xp, 40);
		_screenSurface.drawString("GUY", 4);
		_menu.enableMenuItem(_menu._discussMenu[3]);
	}
	if ((bitIndex & 16) == 16) {
		_screenSurface.putxy(xp, 48);
		_screenSurface.drawString("EVA", 4);
		_menu.enableMenuItem(_menu._discussMenu[4]);
	}
	if ((bitIndex & 8) == 8) {
		_screenSurface.putxy(xp, 56);
		_screenSurface.drawString("BOB", 4);
		_menu.enableMenuItem(_menu._discussMenu[5]);
	}
	if ((bitIndex & 4) == 4) {
		_screenSurface.putxy(xp, 64);
		_screenSurface.drawString("LUC", 4);
		_menu.enableMenuItem(_menu._discussMenu[6]);
	}
	if ((bitIndex & 2) == 2) {
		_screenSurface.putxy(xp, 72);
		_screenSurface.drawString("IDA", 4);
		_menu.enableMenuItem(_menu._discussMenu[7]);
	}
	if ((bitIndex & 1) == 1) {
		_screenSurface.putxy(xp, 80);
		_screenSurface.drawString("MAX", 4);
		_menu.enableMenuItem(_menu._discussMenu[8]);
	}
	_currBitIndex = bitIndex;
}

/**
 * Engine function - Select random characters
 * @remarks	Originally called 'choix'
 */
int MortevielleEngine::selectCharacters(int min, int max) {
	bool invertSelection = false;
	int rand = getRandomNumber(min, max);

	if (rand > 4) {
		rand = 8 - rand;
		invertSelection = true;
	}

	int i = 0;
	int retVal = 0;
	while (i < rand) {
		int charIndex = getRandomNumber(1, 8);
		int charBitIndex = convertCharacterIndexToBitIndex(charIndex);
		if ((retVal & charBitIndex) != charBitIndex) {
			++i;
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
int MortevielleEngine::getPresenceStatsGreenRoom() {
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

	_menu.mdn();

	return retVal;
}
/**
 * Engine function - Get Presence Statistics - Purple Room
 * @remarks	Originally called 'cpl2'
 */
int MortevielleEngine::getPresenceStatsPurpleRoom() {
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
int MortevielleEngine::getPresenceStatsToilets() {
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
int MortevielleEngine::getPresenceStatsBlueRoom() {
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
int MortevielleEngine::getPresenceStatsRedRoom() {
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
void MortevielleEngine::displayAloneText() {
	for (int cf = 1; cf <= 8; ++cf)
		_menu.disableMenuItem(_menu._discussMenu[cf]);

	Common::String sYou = getEngineString(S_YOU);
	Common::String sAre = getEngineString(S_ARE);
	Common::String sAlone = getEngineString(S_ALONE);

	clearScreenType10();
	_screenSurface.putxy(580 - (_screenSurface.getStringWidth(sYou) / 2), 30);
	_screenSurface.drawString(sYou, 4);
	_screenSurface.putxy(580 - (_screenSurface.getStringWidth(sAre) / 2), 50);
	_screenSurface.drawString(sAre, 4);
	_screenSurface.putxy(580 - (_screenSurface.getStringWidth(sAlone) / 2), 70);
	_screenSurface.drawString(sAlone, 4);

	_currBitIndex = 0;
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
	showMouse();
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

/**
 * Engine function - Get Presence Statistics - Room Bureau
 * @remarks	Originally called 'cpl10'
 */
int MortevielleEngine::getPresenceStatsDiningRoom(int &hour) {
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
int MortevielleEngine::getPresenceStatsBureau(int &hour) {
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
int MortevielleEngine::getPresenceStatsKitchen() {
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
int MortevielleEngine::getPresenceStatsAttic() {
	return 0;
}

/**
 * Engine function - Get Presence Statistics - Room Landing
 * @remarks	Originally called 'cpl15'
 */
int MortevielleEngine::getPresenceStatsLanding() {
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
int MortevielleEngine::getPresenceStatsChapel(int &hour) {
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
void MortevielleEngine::setPresenceGreenRoom(int roomId) {
	int rand = getRandomNumber(1, 2);
	if (roomId == GREEN_ROOM) {
		if (rand == 1)
			_roomPresenceLuc = true;
		else
			_roomPresenceIda = true;
	} else if (roomId == DARKBLUE_ROOM) {
		if (rand == 1)
			_roomPresenceGuy = true;
		else
			_roomPresenceEva = true;
	}

	_currBitIndex = 10;
}

/**
 * Engine function - Check who is in the Purple Room
 * @remarks	Originally called 'quelq2'
 */
void MortevielleEngine::setPresencePurpleRoom() {
	if (_place == PURPLE_ROOM)
		_purpleRoomPresenceLeo = true;
	else
		_room9PresenceLeo = true;

	_currBitIndex = 10;
}

/**
 * Engine function - Check who is in the Blue Room
 * @remarks	Originally called 'quelq5'
 */
void MortevielleEngine::setPresenceBlueRoom() {
	_roomPresenceMax = true;
	_currBitIndex = 10;
}

/**
 * Engine function - Check who is in the Red Room
 * @remarks	Originally called 'quelq6'
 */
void MortevielleEngine::setPresenceRedRoom(int roomId) {
	if (roomId == RED_ROOM)
		_roomPresenceBob = true;
	else if (roomId == GREEN_ROOM2)
		_roomPresencePat = true;

	_currBitIndex = 10;
}

/**
 * Engine function - Check who is in the Dining Room
 * @remarks	Originally called 'quelq10'
 */
int MortevielleEngine::setPresenceDiningRoom(int hour) {
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
int MortevielleEngine::setPresenceBureau(int hour) {
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
int MortevielleEngine::setPresenceKitchen() {
	int retVal = chlm();
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Check who is in the Landing
 * @remarks	Originally called 'quelq15'
 */
int MortevielleEngine::setPresenceLanding() {
	bool test = false;
	int rand = 0;
	do {
		rand = getRandomNumber(1, 8);
		test = (((rand == 1) && (_purpleRoomPresenceLeo || _room9PresenceLeo)) ||
		        ((rand == 2) && _roomPresencePat) ||
		        ((rand == 3) && _roomPresenceGuy) ||
		        ((rand == 4) && _roomPresenceEva) ||
		        ((rand == 5) && _roomPresenceBob) ||
		        ((rand == 6) && _roomPresenceLuc) ||
		        ((rand == 7) && _roomPresenceIda) ||
		        ((rand == 8) && _roomPresenceMax));
	} while (test);

	int retVal = convertCharacterIndexToBitIndex(rand);
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Check who is in the chapel
 * @remarks	Originally called 'quelq20'
 */
int MortevielleEngine::setPresenceChapel(int hour) {
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
void MortevielleEngine::getKnockAnswer() {
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

/**
 * Engine function - Get Room Presence Bit Index
 * @remarks	Originally called 'nouvp'
 */
int MortevielleEngine::getPresenceBitIndex(int roomId) {
	int bitIndex = 0;
	if (roomId == GREEN_ROOM) {
		if (_roomPresenceLuc)
			bitIndex = 4;  // LUC
		if (_roomPresenceIda)
			bitIndex = 2;  // IDA
	} else if ( ((roomId == PURPLE_ROOM) && (_purpleRoomPresenceLeo))
			 || ((roomId == ROOM9) && (_room9PresenceLeo)))
		bitIndex = 128;    // LEO
	else if (roomId == DARKBLUE_ROOM) {
		if (_roomPresenceGuy)
			bitIndex = 32; // GUY
		if (_roomPresenceEva)
			bitIndex = 16; // EVA
	} else if ((roomId == BLUE_ROOM) && (_roomPresenceMax))
		bitIndex = 1;      // MAX
	else if ((roomId == RED_ROOM) && (_roomPresenceBob))
		bitIndex = 8;      // BOB
	else if ((roomId == GREEN_ROOM2) && (_roomPresencePat))
		bitIndex = 64;     // PAT
	else if ( ((roomId == TOILETS) && (_toiletsPresenceBobMax))
		   || ((roomId == BATHROOM) && (_bathRoomPresenceBobMax)) )
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

/**
 * Engine function - initGame
 * @remarks	Originally called 'dprog'
 */
void MortevielleEngine::initGame() {
	_place = MANOR_FRONT;
	g_jh = 0;
	if (!g_s._alreadyEnteredManor)
		_blo = true;
	g_t = kTime1;
	g_mh = readclock();
}

/**
 * Engine function - Set Random Presence - Green Room
 * @remarks	Originally called 'pl1'
 */
void MortevielleEngine::setRandomPresenceGreenRoom(int cf) {
	if ( ((_place == GREEN_ROOM) && (!_roomPresenceLuc) && (!_roomPresenceIda))
	  || ((_place == DARKBLUE_ROOM) && (!_roomPresenceGuy) && (!_roomPresenceEva)) ) {
		int p = getPresenceStatsGreenRoom();
		int rand;
		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresenceGreenRoom(_place);
	}
}

/**
 * Engine function - Set Random Presence - Purple Room
 * @remarks	Originally called 'pl2'
 */
void MortevielleEngine::setRandomPresencePurpleRoom(int cf) {
	if (!_purpleRoomPresenceLeo) {
		int p = getPresenceStatsPurpleRoom();
		int rand;
		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresencePurpleRoom();
	}
}

/**
 * Engine function - Set Random Presence - Blue Room
 * @remarks	Originally called 'pl5'
 */
void MortevielleEngine::setRandomPresenceBlueRoom(int cf) {
	if (!_roomPresenceMax) {
		int p = getPresenceStatsBlueRoom();
		int rand;

		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresenceBlueRoom();
	}
}

/**
 * Engine function - Set Random Presence - Red Room
 * @remarks	Originally called 'pl6'
 */
void MortevielleEngine::setRandomPresenceRedRoom(int cf) {
	if ( ((_place == RED_ROOM) && (!_roomPresenceBob))
	  || ((_place == GREEN_ROOM2) && (!_roomPresencePat)) ) {
		int p = getPresenceStatsRedRoom();
		int rand;

		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresenceRedRoom(_place);
	}
}

/**
 * Engine function - Set Random Presence - Room 9
 * @remarks	Originally called 'pl9'
 */
void MortevielleEngine::setRandomPresenceRoom9(int cf) {
	if (!_room9PresenceLeo) {
		cf = -10;
		int p, rand;
		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresencePurpleRoom();
	}
}

/**
 * Engine function - Set Random Presence - Dining Room
 * @remarks	Originally called 'pl10'
 */
void MortevielleEngine::setRandomPresenceDiningRoom(int cf) {
	int h, rand;
	int p = getPresenceStatsDiningRoom(h);
	phaz(rand, p, cf);

	if (rand > p)
		displayAloneText();
	else
		setPresenceDiningRoom(h);
}

/**
 * Engine function - Set Random Presence - Bureau
 * @remarks	Originally called 'pl11'
 */
void MortevielleEngine::setRandomPresenceBureau(int cf) {
	int h, rand;

	int p = getPresenceStatsBureau(h);
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceBureau(h);
}

/**
 * Engine function - Set Random Presence - Kitchen
 * @remarks	Originally called 'pl12'
 */
void MortevielleEngine::setRandomPresenceKitchen(int cf) {
	int p, rand;

	p = getPresenceStatsKitchen();
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceKitchen();
}

/**
 * Engine function - Set Random Presence - Attic / Cellar
 * @remarks	Originally called 'pl13'
 */
void MortevielleEngine::setRandomPresenceAttic(int cf) {
	int p, rand;

	p = getPresenceStatsAttic();
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceKitchen();
}

/**
 * Engine function - Set Random Presence - Landing
 * @remarks	Originally called 'pl15'
 */
void MortevielleEngine::setRandomPresenceLanding(int cf) {
	int p, rand;

	p = getPresenceStatsLanding();
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceLanding();
}

/**
 * Engine function - Set Random Presence - Chapel
 * @remarks	Originally called 'pl20'
 */
void MortevielleEngine::setRandomPresenceChapel(int cf) {
	int h, rand;

	int p = getPresenceStatsChapel(h);
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceChapel(h);
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
	clearScreenType3();
	g_vm->prepareScreenType2();
	ecr3(g_vm->getEngineString(S_SOMEONE_ENTERS));
	int rand = (getRandomNumber(0, 4)) - 2;
	g_vm->_speechManager.startSpeech(2, rand, 1);

	// The original was doing here a useless loop.
	// It has been removed

	clearScreenType3();
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

/**
 * Start music or speech
 * @remarks	Originally called 'musique'
 */
void MortevielleEngine::startMusicOrSpeech(int so) {
	if (so == 0) {
		/* musik(0) */
		;
	} else if ((g_prebru == 0) && (!g_s._alreadyEnteredManor)) {
		// Type 1: Speech
		_speechManager.startSpeech(10, 1, 1);
		++g_prebru;
	} else {
		if (((g_s._currPlace == MOUNTAIN) || (g_s._currPlace == MANOR_FRONT) || (g_s._currPlace == MANOR_BACK)) && (getRandomNumber(1, 3) == 2))
			// Type 1: Speech
			_speechManager.startSpeech(9, getRandomNumber(2, 4), 1);
		else if ((g_s._currPlace == CHAPEL) && (getRandomNumber(1, 2) == 1))
			// Type 1: Speech
			_speechManager.startSpeech(8, 1, 1);
		else if ((g_s._currPlace == WELL) && (getRandomNumber(1, 2) == 2))
			// Type 1: Speech
			_speechManager.startSpeech(12, 1, 1);
		else if (g_s._currPlace == INSIDE_WELL)
			// Type 1: Speech
			_speechManager.startSpeech(13, 1, 1);
		else
			// Type 2 : music
			_speechManager.startSpeech(getRandomNumber(1, 17), 1, 2);
	}
}

/* NIVEAU 9 */
void dessin(int ad) {
	if (ad != 0)
		dessine(g_ades, ((ad % 160) * 2), (ad / 160));
	else {
		clearScreenType1();
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
							clearScreenType3();
							g_vm->prepareScreenType2();
							ecr3(g_vm->getEngineString(S_HEAR_NOISE));
							int rand = (getRandomNumber(0, 4)) - 2;
							g_vm->_speechManager.startSpeech(1, rand, 1);
							clearScreenType3();
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
void afdes() {
	taffich();
	dessin(0);
	g_vm->_okdes = false;
}

void tkey1(bool d) {
	bool quest = false;
	int x, y, c;

	hideMouse();
	fenat('K');

	// Wait for release from any key or mouse button
	while (g_vm->keyPressed())
		g_key = testou();
	do {
		getMousePos_(x, y, c);
		g_vm->keyPressed();
	} while (c != 0);
	
	// Event loop
	do {
		if (d)
			tinke();
		quest = g_vm->keyPressed();
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

/**
 * Engine function - You lose!
 * @remarks	Originally called 'tperd'
 */
void MortevielleEngine::loseGame() {
	initouv();
	g_ment = 0;
	g_iouv = 0;
	g_mchai = 0;
	_menu.unsetSearchMenu();
	if (!_blo)
		t11(MANOR_FRONT);

	_loseGame = true;
	clearScreenType1();
	_screenSurface.drawBox(60, 35, 400, 50, 15);
	repon(9, g_crep);
	clearScreenType2();
	clearScreenType3();
	_col = false;
	_syn = false;
	_okdes = false;
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

/**
 * Engine function - Check inventory for a given object
 * @remarks	Originally called 'cherjer'
 */
bool MortevielleEngine::checkInventory(int objectId) {
	bool retVal = false;
	for (int i = 1; i <= 6; ++i)
		retVal = (retVal || (ord(g_s._sjer[i]) == objectId));

	if (g_s._selectedObjectId == objectId)
		retVal = true;

	return retVal;
}

/**
 * Engine function - Display Dining Room
 * @remarks	Originally called 'st1sama'
 */
void MortevielleEngine::displayDiningRoom() {
	g_s._currPlace = DINING_ROOM;
	affrep();
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
		g_vm->_speechManager.startSpeech(repint, haut[g_caff - 69], 0);
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

/**
 * Engine function - Go to Dining room
 * @remarks	Originally called 't1sama'
 */
void MortevielleEngine::gotoDiningRoom() {
	int day, hour, minute;

	updateHour(day, hour, minute);
	if ((hour < 5) && (g_s._currPlace > ROOM18)) {
		if (!checkInventory(137)) {        //You don't have the keys, and it's late
			g_crep = 1511;
			loseGame();
		} else
			displayDiningRoom();
	} else if (!g_s._alreadyEnteredManor) {     //Is it your first time?
		_currBitIndex = 255; // Everybody is present
		showPeoplePresent(_currBitIndex);
		g_caff = 77;
		afdes();
		_screenSurface.drawBox(223, 47, 155, 91, 15);
		repon(2, 33);
		tkey1(false);
		mennor();
		hideMouse();
		hirs();
		premtet();
		sparl(0, 140);
		drawRightFrame();
		drawClock();
		showMouse();
		g_s._currPlace = OWN_ROOM;
		affrep();
		resetPresenceInRooms(DINING_ROOM);
		if (!_blo)
			minute = t11(OWN_ROOM);
		_currBitIndex = 0;
		g_mpers = 0;
		g_s._alreadyEnteredManor = true;
	} else
		displayDiningRoom();
}

/**
 * Engine function - Check Manor distance (in the mountains)
 * @remarks	Originally called 't1neig'
 */
void MortevielleEngine::checkManorDistance() {
	++_manorDistance;
	if (_manorDistance > 2) {
		g_crep = 1506;
		loseGame();
	} else {
		_okdes = true;
		g_s._currPlace = MOUNTAIN;
		affrep();
	}
}

/**
 * Engine function - Go to Manor front
 * @remarks	Originally called 't1deva'
 */
void MortevielleEngine::gotoManorFront() {
	_manorDistance = 0;
	g_s._currPlace = MANOR_FRONT;
	affrep();
}

/**
 * Engine function - Go to Manor back
 * @remarks	Originally called 't1derr'
 */
void MortevielleEngine::gotoManorBack() {
	g_s._currPlace = MANOR_BACK;
	affrep();
}

/**
 * Engine function - Dead : Flooded in Well 
 * @remarks	Originally called 't1deau'
 */
void MortevielleEngine::floodedInWell() {
	g_crep = 1503;
	loseGame();
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
		int cx = convertBitIndexToCharacterIndex(g_vm->_currBitIndex);
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
			clearScreenType2();
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
			Alert::show(g_vm->_hintPctMessage, 1);
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

/**
 * Engine function - Change Graphical Device
 * @remarks	Originally called 'change_gd'
 */
void MortevielleEngine::changeGraphicalDevice(int newDevice) {
	hideMouse();
	_currGraphicalDevice = newDevice;
	hirs();
	initMouse();
	showMouse();
	drawRightFrame();
	tinke();
	drawClock();
	if (_currBitIndex != 0)
		showPeoplePresent(_currBitIndex);
	else
		displayAloneText();
	clearScreenType2();
	clearScreenType3();
	g_maff = 68;
	afdes();
	repon(2, g_crep);
	_menu.displayMenu();
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
	g_mnumo = OPCODE_NONE;
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
	_manorDistance = 0;
	initouv();
	g_iouv = 0;
	g_dobj = 0;
	affrep();
	_hintPctMessage = deline(580);

	_okdes = false;
	_endGame = true;
	_loseGame = false;
	_heroSearching = false;

	displayAloneText();
	tinke();
	drawClock();
	afdes();
	repon(2, g_crep);
	clearScreenType3();
	_endGame = false;
	_menu.setDestinationMenuText(g_s._currPlace);
	modinv();
	if (g_s._selectedObjectId != 0)
		modobj(g_s._selectedObjectId + 400);
	showMouse();
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
	if (((g_s._currPlace < CRYPT) || (g_s._currPlace > MOUNTAIN)) && (g_s._currPlace != INSIDE_WELL)
	        && (g_s._currPlace != OWN_ROOM) && (g_s._selectedObjectId != 152) && (!_loseGame)) {
		if ((g_s._faithScore > 99) && (hour > 8) && (hour < 16)) {
			g_crep = 1501;
			loseGame();
		}
		if ((g_s._faithScore > 99) && (hour > 0) && (hour < 9)) {
			g_crep = 1508;
			loseGame();
		}
		if ((day > 1) && (hour > 8) && (!_loseGame)) {
			g_crep = 1502;
			loseGame();
		}
	}
	mennor();
}

/**
 * Engine function - Transform time into a char
 * @remarks	Originally called 'tmaj3'
 */
void MortevielleEngine::hourToChar() {
	int day, hour, minute;

	g_vm->updateHour(day, hour, minute);
	if (minute == 30)
		minute = 1;
	hour += day * 24;
	minute += hour * 2;
	g_s._fullHour = chr(minute);
}

/**
 * Engine function - extract time from a char
 * @remarks	Originally called 'theure'
 */
void MortevielleEngine::charToHour() {
	int fullHour = ord(g_s._fullHour);
	int tmpHour = fullHour % 48;
	g_vm->_currDay = fullHour / 48;
	g_vm->_currHalfHour = tmpHour % 2;
	g_vm->_currHour = tmpHour / 2;
	g_vm->_hour = g_vm->_currHour;
	if (g_vm->_currHalfHour == 1)
		g_vm->_minute = 30;
	else
		g_vm->_minute = 0;
}
} // End of namespace Mortevielle
