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

#include "mortevielle/var_mor.h"

namespace Mortevielle {

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
extern void tmlieu(int mli);
/* NIVEAU 7 */
extern void tlu(int af, int ob);
extern void affrep();
extern void mfouen();
/* NIVEAU 6 */
extern void tperd();
extern void tsort();
extern void st4(int ob);
extern void cherjer(int ob, bool &d);
extern void st1sama();
extern void modinv();
extern void sparl(float adr, float rep);
extern void finfouil();
extern void mfoudi();
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

} // End of namespace Mortevielle
#endif
