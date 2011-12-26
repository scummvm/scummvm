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

#ifndef MORTEVIELLE_MOR2_H
#define MORTEVIELLE_MOR2_H

namespace Mortevielle {

void tinke();
void fenat(char ans);
/* NIVEAU 8 */
void afdes(int ad);
void tkey1(bool d);
void tmlieu(int mli);
/* NIVEAU 7 */
void tlu(int af, int ob);
void delin(int n);
void affrep();
void mfouen();
void atf3f8(int &key);
/* NIVEAU 6 */
void tperd();
void tsort();
void st4(int ob);
void cherjer(int ob, bool &d);
void st1sama();
void modinv();
void sparl(long_int adr, long_int rep);
void finfouil();
void mfoudi();
void mennor();
void premtet();
/* NIVEAU 5 */
void ajchai();
void ajjer(int ob);
void t1sama();
void t1vier();
void t1neig();
void t1deva();
void t1derr();
void t1deau();
void tctrm();
void quelquun();
void tsuiv();
void tfleche();
void tcoord(int sx);
void st7(int ob);
void treg(int ob);
void avpoing(int &ob);
void rechai(int &ch);
void t23coul(int &l);
void maivid();
void st13(int ob);
void aldepl();

} // End of namespace Mortevielle
#endif
