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

#ifndef MORTEVIELLE_LEVEL15_H
#define MORTEVIELLE_LEVEL15_H

#include "mortevielle/var_mor.h"

namespace Mortevielle {

/* NIVEAU 15 */
extern void copcha();
extern bool dans_rect(rectangle r);
extern void outbloc(int n, pattern p, t_nhom pal);
extern void writepal(int n);
extern void pictout(int seg, int dep, int x, int y);
extern void putxy(int x, int y);
// extern int calad(int x, int y);
extern void sauvecr(int y, int dy);
extern void charecr(int y, int dy);
// extern int peek(int ad);
// extern int peekw(int ad);
// extern real peekl(int ad);
// extern void procedure poke(int ad, int n);
extern void adzon();
extern int animof(int ouf, int num);
// extern void affgd(int offs, int dep, int x, int y);

} // End of namespace Mortevielle
#endif
