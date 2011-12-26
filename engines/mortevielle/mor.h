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
void testfi();
int readclock();
void modif(int &nu);
void dessine(int ad, int x, int y);
void dessine_rouleau();
void text_color(int c);
/* NIVEAU 13 */
void text1(int x, int y, int nb, int m);
void initouv();
void ecrf1();
void clsf1();
void clsf2();
void ecrf2();
void ecr2(Common::String str_);
void clsf3();
void ecr3(Common::String text);
void ecrf6();
void ecrf7();
void clsf10();
void stop();
void paint_rect(int x, int y, int dx, int dy);
int hazard(int min, int max);
void calch(int &j, int &h, int &m);
void conv(int x, int &y);
/* NIVEAU 12 */
void okpas();
void modobj(int m);
void modobj2(int m, bool t1, bool t2);
void repon(int f, int m);
void f3f8();
void t5(int cx);
void affper(int per);
void choix(int min, int max, int &per);
void cpl1(int &p);
void cpl2(int &p);
void cpl3(int &p);
void cpl5(int &p);
void cpl6(int &p);
void person();
void chlm(int &per);
void pendule();
/*************
 * NIVEAU 11 *
 *************/
void debloc(int l);
void cpl10(int &p, int &h);
void cpl11(int &p, int &h);
void cpl12(int &p);
void cpl13(int &p);
void cpl15(int &p);
void cpl20(int &p, int &h);
void quelq1(int l);
void quelq2();
void quelq5();
void quelq6(int l);
void quelq10(int h, int &per);
void quelq11(int h, int &per);
void quelq12(int &per);
void quelq15(int &per);
void quelq20(int h, int &per);
void frap();
void nouvp(int l, int &p);
void tip(int ip, int &cx);
void ecfren(int &p, int &haz, int cf, int l);
void becfren(int l);
/* NIVEAU 10 */
void init_nbrepm();
void phaz(int &haz, int &p, int cf);
void inzon();
void dprog();
void pl1(int cf);
void pl2(int cf);
void pl5(int cf);
void pl6(int cf);
void pl9(int cf);
void pl10(int cf);
void pl11(int cf);
void pl12(int cf);
void pl13(int cf);
void pl15(int cf);
void pl20(int cf);
void t11(int l11, int &a);
void cavegre();
void writetp(Common::String s, int t);
void messint(int nu);
void aniof(int ouf, int num);
void musique(int so);
/* NIVEAU 9 */
void dessin(int ad);

} // End of namespace Mortevielle
#endif
