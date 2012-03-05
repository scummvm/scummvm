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

namespace Mortevielle {

/* Niveau 14 suite */
extern void testfi();
extern int readclock();
extern void modif(int &nu);
extern void dessine(int ad, int x, int y);
extern void dessine_rouleau();
extern void text_color(int c);
/* NIVEAU 13 */
extern void text1(int x, int y, int nb, int m);
extern void initouv();
extern void ecrf1();
extern void clsf1();
extern void clsf2();
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
extern void conv(int x, int &y);
/* NIVEAU 12 */
extern void modobj(int m);
extern void repon(int f, int m);
extern void t5(int cx);
extern void showPeoplePresent(int per);
extern void choix(int min, int max, int &per);
extern int cpl1();
extern int cpl2();
extern int cpl3();
extern int cpl5();
extern int cpl6();
extern void displayAloneText();
extern int chlm();
extern void drawClock();
/*************
 * NIVEAU 11 *
 *************/
extern void debloc(int l);
extern void cpl10(int &p, int &hour);
extern void cpl11(int &p, int &hour);
extern int  cpl12();
extern int  cpl13();
extern int  cpl15();
extern void cpl20(int &p, int &hour);
extern void setPresenceGreenRoom(int l);
extern void setPresencePurpleRoom();
extern void setPresenceBlueRoom();
extern void setPresenceRedRoom(int l);
extern int setPresenceDiningRoom(int hour);
extern int setPresenceBureau(int hour);
extern int setPresenceKitchen();
extern int setPresenceLanding();
extern int setPresenceChapel(int hour);
extern void frap();
extern void nouvp(int l, int &p);
extern void tip(int ip, int &cx);
extern void ecfren(int &p, int &rand, int cf, int l);
extern void becfren(int l);
/* NIVEAU 10 */
extern void init_nbrepm();
extern void phaz(int &rand, int &p, int cf);
extern void inzon();
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
extern void t11(int l11, int &a);
extern void cavegre();
extern void writetp(Common::String s, int t);
extern void aniof(int ouf, int num);
extern void musique(int so);
/* NIVEAU 9 */
extern void dessin(int ad);

} // End of namespace Mortevielle
#endif
