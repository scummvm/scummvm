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

/* NIVEAU 10 */
void phaz(int &rand, int &p, int cf) {
	p += cf;
	rand = g_vm->getRandomNumber(1, 100);
}

int t11(int roomId) {
	int retVal = 0;
	int rand;

	int p = g_vm->getPresenceStats(rand, g_vm->_coreVar._faithScore, roomId);
	g_vm->_place = roomId;
	if ((roomId > OWN_ROOM) && (roomId < DINING_ROOM)) {
		if (p != -500) {
			if (rand > p) {
				g_vm->displayAloneText();
				retVal = 0;
			} else {
				g_vm->setPresenceFlags(g_vm->_place);
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
			p += g_vm->_coreVar._faithScore;
			rand = g_vm->getRandomNumber(1, 100);
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

void writetp(Common::String s, int t) {
	if (g_vm->_res == 2)
		g_vm->_screenSurface.drawString(s, t);
	else
		g_vm->_screenSurface.drawString(g_vm->copy(s, 1, 25), t);
}

void aniof(int ouf, int num) {
	if ((g_vm->_caff == 7) && ((num == 4) || (num == 5)))
		return;
	
	if ((g_vm->_caff == 10) && (num == 7))
		num = 6;
	else if (g_vm->_caff == 12) {
		if (num == 3)
			num = 4;
		else if (num == 4)
			num = 3;
	}

	int ad = kAdrAni;
	int offset = g_vm->animof(ouf, num);

	GfxSurface surface;
	surface.decode(&g_vm->_mem[ad * 16 + offset]);
	g_vm->_screenSurface.drawPicture(surface, 0, 12);

	g_vm->prepareScreenType1();
}

/* NIVEAU 9 */
void dessin(int ad) {
	if (ad != 0)
		g_vm->draw(kAdrDes, ((ad % 160) * 2), (ad / 160));
	else {
		g_vm->clearScreenType1();
		if (g_vm->_caff > 99) {
			g_vm->draw(kAdrDes, 60, 33);
			g_vm->_screenSurface.drawBox(118, 32, 291, 121, 15);         // Medium box
		} else if (g_vm->_caff > 69) {
			g_vm->draw(kAdrDes, 112, 48);           // Heads
			g_vm->_screenSurface.drawBox(222, 47, 155, 91, 15);
		} else {
			g_vm->draw(kAdrDes, 0, 12);
			g_vm->prepareScreenType1();
			if ((g_vm->_caff < 30) || (g_vm->_caff > 32)) {
				for (int cx = 1; cx <= 6; ++cx) {
					if (ord(g_vm->_touv[cx]) != 0)
						aniof(1, ord(g_vm->_touv[cx]));
				}

				if (g_vm->_caff == 13) {
					if (g_vm->_coreVar._atticBallHoleObjectId == 141)
						aniof(1, 7);

					if (g_vm->_coreVar._atticRodHoleObjectId == 159)
						aniof(1, 6);
				} else if ((g_vm->_caff == 14) && (g_vm->_coreVar._cellarObjectId == 151))
					aniof(1, 2);
				else if ((g_vm->_caff == 17) && (g_vm->_coreVar._secretPassageObjectId == 143))
					aniof(1, 1);
				else if ((g_vm->_caff == 24) && (g_vm->_coreVar._wellObjectId != 0))
					aniof(1, 1);
			}
			
			if (g_vm->_caff < ROOM26)
				g_vm->startMusicOrSpeech(1);
		}
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
		g_vm->_key = testou();

	do {
		g_vm->_mouse.getMousePosition(x, y, c);
		g_vm->keyPressed();
	} while (c != 0);
	
	// Event loop
	do {
		if (d)
			g_vm->prepareRoom();
		quest = g_vm->keyPressed();
		g_vm->_mouse.getMousePosition(x, y, c);
		CHECK_QUIT;
	} while (!(quest || (c != 0) || (d && g_vm->_anyone)));
	if (quest)
		testou();
	g_vm->setMouseClick(false);
	g_vm->_mouse.showMouse();
}

/* NIVEAU 7 */
void tlu(int af, int ob) {
	g_vm->_caff = 32;
	afdes();
	g_vm->repon(6, ob + 4000);
	g_vm->repon(2, 999);
	tkey1(true);
	g_vm->_caff = af;
	g_vm->_msg[3] = OPCODE_NONE;
	g_vm->_crep = 998;
}

void affrep() {
	g_vm->_caff = g_vm->_coreVar._currPlace;
	g_vm->_crep = g_vm->_coreVar._currPlace;
}

void tsort() {

	if ((g_vm->_iouv > 0) && (g_vm->_coreVar._currPlace != 0)) {
		if (g_vm->_coreVar._faithScore < 50)
			g_vm->_coreVar._faithScore += 2;
		else
			g_vm->_coreVar._faithScore += (g_vm->_coreVar._faithScore / 10);
	}

	for (int cx = 1; cx <= 7; ++cx)
		g_vm->_touv[cx] = chr(0);
	g_vm->_ment = 0;
	g_vm->_iouv = 0;
	g_vm->_mchai = 0;
	g_vm->resetRoomVariables(g_vm->_coreVar._currPlace);
}

void st4(int ob) {
	g_vm->_crep = 997;

	switch (ob) {
	case 114 :
		g_vm->_crep = 109;
		break;
	case 110 :
		g_vm->_crep = 107;
		break;
	case 158 :
		g_vm->_crep = 113;
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
		tlu(g_vm->_caff, ob);
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
		if (g_vm->_coreVar._sjer[i] != chr(0)) {
			++cy;
			r = (ord(g_vm->_coreVar._sjer[i]) + 400);
			nomp = g_vm->getString(r - 501 + kInventoryStringIndex);
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
	g_vm->_menu.menuUp(g_vm->_msg[3]);
}

void premtet() {
	g_vm->draw(kAdrDes, 10, 80);
	g_vm->_screenSurface.drawBox(18, 79, 155, 91, 15);
}

/* NIVEAU 5 */
void ajchai() {
	int cy = kAcha + ((g_vm->_mchai - 1) * 10) - 1;
	int cx = 0;
	do {
		++cx;
	} while ((cx <= 9) && (g_vm->_tabdon[cy + cx] != 0));

	if (g_vm->_tabdon[cy + cx] == 0)
		g_vm->_tabdon[cy + cx] = g_vm->_coreVar._selectedObjectId;
	else
		g_vm->_crep = 192;
}

void ajjer(int ob) {
	int cx = 0;
	do {
		++cx;
	} while ((cx <= 5) && (ord(g_vm->_coreVar._sjer[cx]) != 0));

	if (ord(g_vm->_coreVar._sjer[cx]) == 0) {
		g_vm->_coreVar._sjer[(cx)] = chr(ob);
		modinv();
	} else
		g_vm->_crep = 139;
}

void quelquun() {
	if (g_vm->_menu._menuDisplayed)
		g_vm->_menu.eraseMenu();

	g_vm->endSearch();
	g_vm->_crep = 997;
L1:
	if (!g_vm->_hiddenHero) {
		if (g_vm->_crep == 997)
			g_vm->_crep = 138;
		g_vm->repon(2, g_vm->_crep);
		if (g_vm->_crep == 138)
			g_vm->_speechManager.startSpeech(5, 2, 1);
		else
			g_vm->_speechManager.startSpeech(4, 4, 1);

		if (g_vm->_iouv == 0)
			g_vm->_coreVar._faithScore += 2;
		else if (g_vm->_coreVar._faithScore < 50)
			g_vm->_coreVar._faithScore += 4;
		else
			g_vm->_coreVar._faithScore += 3 * (g_vm->_coreVar._faithScore / 10);
		tsort();
		g_vm->_menu.setDestinationMenuText(LANDING);
		int cx = g_vm->convertBitIndexToCharacterIndex(g_vm->_currBitIndex);
		g_vm->_caff = 69 + cx;
		g_vm->_crep = g_vm->_caff;
		g_vm->_msg[3] = MENU_DISCUSS;
		g_vm->_msg[4] = g_vm->_menu._discussMenu[cx];
		g_vm->_syn = true;
		g_vm->_col = true;
	} else {
		if (g_vm->getRandomNumber(1, 3) == 2) {
			g_vm->_hiddenHero = false;
			g_vm->_crep = 137;
			goto L1;
		} else {
			g_vm->repon(2, 136);
			int rand = (g_vm->getRandomNumber(0, 4)) - 2;
			g_vm->_speechManager.startSpeech(3, rand, 1);
			g_vm->clearScreenType2();
			g_vm->displayAloneText();
			g_vm->resetRoomVariables(MANOR_FRONT);
			affrep();
		}
	}
	if (g_vm->_menu._menuDisplayed)
		g_vm->_menu.drawMenu();
}

void tsuiv() {
	int tbcl;
	int cl;

	int cy = kAcha + ((g_vm->_mchai - 1) * 10) - 1;
	int cx = 0;
	do {
		++cx;
		++g_vm->_cs;
		cl = cy + g_vm->_cs;
		tbcl = g_vm->_tabdon[cl];
	} while ((tbcl == 0) && (g_vm->_cs <= 9));

	if ((tbcl != 0) && (g_vm->_cs < 11)) {
		++g_vm->_is;
		g_vm->_caff = tbcl;
		g_vm->_crep = g_vm->_caff + 400;
		if (g_vm->_currBitIndex != 0)
			g_vm->_coreVar._faithScore += 2;
	} else {
		affrep();
		g_vm->endSearch();
		if (cx > 9)
			g_vm->_crep = 131;
	}
}

void tfleche() {
	bool qust;
	char touch;

	if (g_vm->_num == 9999)
		return;

	fenat(chr(152));
	bool inRect = false;
	do {
		touch = '\0';

		do {
			g_vm->_mouse.moveMouse(qust, touch);
			CHECK_QUIT;

			if (g_vm->getMouseClick())
				inRect = (g_vm->_mouse._pos.x < 256 * g_vm->_res) && (g_vm->_mouse._pos.y < 176) && (g_vm->_mouse._pos.y > 12);
			g_vm->prepareRoom();
		} while (!(qust || inRect || g_vm->_anyone));

		if (qust && (touch == '\103'))
			Alert::show(g_vm->_hintPctMessage, 1);
	} while (!((touch == '\73') || ((touch == '\104') && (g_vm->_x != 0) && (g_vm->_y != 0)) || (g_vm->_anyone) || (inRect)));

	if (touch == '\73')
		g_vm->_keyPressedEsc = true;

	if (inRect) {
		g_vm->_x = g_vm->_mouse._pos.x;
		g_vm->_y = g_vm->_mouse._pos.y;
	}
}

void tcoord(int sx) {
	int sy, ix, iy;
	int ib;


	g_vm->_num = 0;
	g_vm->_crep = 999;
	int a = 0;
	int atdon = amzon + 3;
	int cy = 0;
	while (cy < g_vm->_caff) {
		a += g_vm->_tabdon[atdon];
		atdon += 4;
		++cy;
	}

	if (g_vm->_tabdon[atdon] == 0) {
		g_vm->_crep = 997;
		return;
	}

	a += kFleche;
	int cb = 0;
	for (cy = 0; cy <= (sx - 2); ++cy) {
		ib = (g_vm->_tabdon[a + cb] << 8) + g_vm->_tabdon[(a + cb + 1)];
		cb += (ib * 4) + 2;
	}
	ib = (g_vm->_tabdon[a + cb] << 8) + g_vm->_tabdon[(a + cb + 1)];
	if (ib == 0) {
		g_vm->_crep = 997;
		return;
	}

	cy = 1;
	do {
		cb += 2;
		sx = g_vm->_tabdon[a + cb] * g_vm->_res;
		sy = g_vm->_tabdon[(a + cb + 1)];
		cb += 2;
		ix = g_vm->_tabdon[a + cb] * g_vm->_res;
		iy = g_vm->_tabdon[(a + cb + 1)];
		++cy;
	} while (!(((g_vm->_x >= sx) && (g_vm->_x <= ix) && (g_vm->_y >= sy) && (g_vm->_y <= iy)) || (cy > ib)));

	if ((g_vm->_x >= sx) && (g_vm->_x <= ix) && (g_vm->_y >= sy) && (g_vm->_y <= iy)) {
		g_vm->_num = cy - 1;
		return;
	}

	g_vm->_crep = 997;
}


void st7(int ob) {
	switch (ob) {
	case 116:
	case 144:
		g_vm->_crep = 104;
		break;
	case 126:
	case 111:
		g_vm->_crep = 108;
		break;
	case 132:
		g_vm->_crep = 111;
		break;
	case 142:
		g_vm->_crep = 112;
		break;
	default:
		g_vm->_crep = 183;
		st4(ob);
	}
}

void treg(int ob) {
	int mdes = g_vm->_caff;
	g_vm->_caff = ob;

	if (((g_vm->_caff > 29) && (g_vm->_caff < 33)) || (g_vm->_caff == 144) || (g_vm->_caff == 147) || (g_vm->_caff == 149) || (g_vm->_msg[4] == OPCODE_SLOOK)) {
		afdes();
		if ((g_vm->_caff > 29) && (g_vm->_caff < 33))
			g_vm->repon(2, g_vm->_caff);
		else
			g_vm->repon(2, g_vm->_caff + 400);
		tkey1(true);
		g_vm->_caff = mdes;
		g_vm->_msg[3] = 0;
		g_vm->_crep = 998;
	} else {
		g_vm->_obpart = true;
		g_vm->_crep = g_vm->_caff + 400;
		g_vm->_menu.setSearchMenu();
	}
}

void avpoing(int &ob) {
	g_vm->_crep = 999;
	if (g_vm->_coreVar._selectedObjectId != 0)
		ajjer(g_vm->_coreVar._selectedObjectId);

	if (g_vm->_crep != 139) {
		g_vm->displayItemInHand(ob + 400);
		g_vm->_coreVar._selectedObjectId = ob;
		ob = 0;
	}
}

void rechai(int &ch) {
	int tmpPlace = g_vm->_coreVar._currPlace;

	if (g_vm->_coreVar._currPlace == CRYPT)
		tmpPlace = CELLAR;
	ch = g_vm->_tabdon[achai + (tmpPlace * 7) + g_vm->_num - 1];
}

int t23coul() {
	if (!g_vm->checkInventory(143)) {
		g_vm->_crep = 1512;
		g_vm->loseGame();
	}

	return CELLAR;
}

void st13(int ob) {
	if ((ob == 114) || (ob == 116) || (ob == 126) || (ob == 132) ||
	    (ob == 111) || (ob == 106) || (ob == 102) || (ob == 100) ||
	    (ob == 110) || (ob == 153) || (ob == 154) || (ob == 155) ||
	    (ob == 156) || (ob == 157) || (ob == 144) || (ob == 158) ||
	    (ob == 150) || (ob == 152))
		g_vm->_crep = 999;
	else
		g_vm->_crep = 105;
}

void sauvecr(int y, int dy) {
//	g_vm->_mouse.hideMouse();
//	g_vm->_mouse.showMouse();
}

void charecr(int y, int dy) {
//	g_vm->_mouse.hideMouse();
//	g_vm->_mouse.showMouse();
}

} // End of namespace Mortevielle
