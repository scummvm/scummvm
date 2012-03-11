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

#ifndef MORTEVIELLE_MOR_H
#define MORTEVIELLE_MOR_H

#include "common/str.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

const int kTime1 = 410;
const int kTime2 = 250;

/* NIVEAU 15 */
extern void copcha();
extern bool isMouseIn(rectangle r);
extern void outbloc(int n, pattern p, nhom *pal);
extern void writepal(int n);
extern void pictout(int seg, int dep, int x, int y);
extern void sauvecr(int y, int dy);
extern void charecr(int y, int dy);
extern void adzon();
extern int animof(int ouf, int num);
/* Niveau 14 suite */
extern int readclock();
extern void modif(int &nu);
extern void dessine(int ad, int x, int y);
extern void dessine_rouleau();
extern void setTextColor(int c);
/* NIVEAU 13 */
extern void text1(int x, int y, int nb, int m);
extern void initouv();
extern void ecrf1();
extern void clsf1();
extern void clearScreenType2();
extern void ecrf2();
extern void ecr2(Common::String str_);
extern void clsf3();
extern void ecr3(Common::String text);
extern void ecrf6();
extern void ecrf7();
extern void clsf10();
extern void stop();
extern void paint_rect(int x, int y, int dx, int dy);
extern void updateHour(int &day, int &hour, int &minute);
extern int convertCharacterIndexToBitIndex(int characterIndex);
/* NIVEAU 12 */
extern void modobj(int m);
extern void repon(int f, int m);
extern void t5(int cx);
extern void showPeoplePresent(int per);
extern int selectCharacters(int min, int max);
extern int getPresenceStatsGreenRoom();
extern int getPresenceStatsPurpleRoom();
extern int getPresenceStatsToilets();
extern int getPresenceStatsBlueRoom();
extern int getPresenceStatsRedRoom();
extern void displayAloneText();
extern int chlm();
extern void drawClock();
/*************
 * NIVEAU 11 *
 *************/
extern void debloc(int l);
extern int  getPresenceStatsDiningRoom(int &hour);
extern int  getPresenceStatsBureau(int &hour);
extern int  getPresenceStatsKitchen();
extern int  getPresenceStatsAttic();
extern int  getPresenceStatsLanding();
extern int  getPresenceStatsChapel(int &hour);
extern void setPresenceGreenRoom(int roomId);
extern void setPresencePurpleRoom();
extern void setPresenceBlueRoom();
extern void setPresenceRedRoom(int l);
extern int setPresenceDiningRoom(int hour);
extern int setPresenceBureau(int hour);
extern int setPresenceKitchen();
extern int setPresenceLanding();
extern int setPresenceChapel(int hour);
extern void getKnockAnswer();
extern int nouvp(int l);
extern int convertBitIndexToCharacterIndex(int bitIndex);
extern void ecfren(int &p, int &rand, int cf, int l);
extern void becfren(int l);
/* NIVEAU 10 */
extern void init_nbrepm();
extern void phaz(int &rand, int &p, int cf);
extern void resetVariables();
extern void dprog();
extern void pl1(int cf);
extern void pl2(int cf);
extern void pl5(int cf);
extern void pl6(int cf);
extern void pl9(int cf);
extern void pl10(int cf);
extern void pl11(int cf);
extern void pl12(int cf);
extern void pl13(int cf);
extern void pl15(int cf);
extern void pl20(int cf);
extern int t11(int l11);
extern void cavegre();
extern void writetp(Common::String s, int t);
extern void aniof(int ouf, int num);
extern void musique(int so);
/* NIVEAU 9 */
extern void dessin(int ad);

static const int _actionMenu[12] = { OPCODE_NONE,
		OPCODE_SHIDE, OPCODE_ATTACH, OPCODE_FORCE, OPCODE_SLEEP, 
		OPCODE_ENTER, OPCODE_CLOSE,  OPCODE_KNOCK, OPCODE_EAT,
		OPCODE_PLACE, OPCODE_OPEN,   OPCODE_LEAVE
};

extern void tinke();
extern void fenat(char ans);
/* NIVEAU 8 */
extern void afdes(int ad);
extern void tkey1(bool d);
extern void tmlieu(int roomId);
/* NIVEAU 7 */
extern void tlu(int af, int ob);
extern void affrep();
extern void unsetSearchMenu();
/* NIVEAU 6 */
extern void tperd();
extern void tsort();
extern void st4(int ob);
extern void cherjer(int ob, bool &d);
extern void st1sama();
extern void modinv();
extern void sparl(float adr, float rep);
extern void endSearch();
extern void setSearchMenu();
extern void mennor();
extern void premtet();
/* NIVEAU 5 */
extern void ajchai();
extern void ajjer(int ob);
extern void t1sama();
extern void t1vier();
extern void t1neig();
extern void t1deva();
extern void t1derr();
extern void t1deau();
extern void tctrm();
extern void quelquun();
extern void tsuiv();
extern void tfleche();
extern void tcoord(int sx);
extern void st7(int ob);
extern void treg(int ob);
extern void avpoing(int &ob);
extern void rechai(int &ch);
extern void t23coul(int &l);
extern void maivid();
extern void st13(int ob);
extern void aldepl();

extern void changeGraphicalDevice(int newDevice);

/* NIVEAU 3 */
/* procedure PROGRAMME */
extern void tmaj3();

/* NIVEAU 1 */
extern void theure();
} // End of namespace Mortevielle
#endif
