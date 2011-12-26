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

namespace Mortevielle {

/* NIVEAU 15 */
void copcha();
bool dans_rect(rectangle r);
void outbloc(int n, pattern p, t_nhom pal);
void writepal(int n);
void pictout(int seg, int dep, int x, int y);
void putxy(int x, int y);
// int calad(int x, int y);
void sauvecr(int y, int dy);
void charecr(int y, int dy);
// int peek(int ad);
// int peekw(int ad);
// real peekl(int ad);
// void procedure poke(int ad, int n);
void adzon();
int animof(int ouf, int num);
// void affgd(int offs, int dep, int x, int y);

} // End of namespace Mortevielle
#endif
