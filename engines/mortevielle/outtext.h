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

#ifndef MORTEVIELLE_OUTTEXT_H
#define MORTEVIELLE_OUTTEXT_H

#include "mortevielle/var_mor.h"

namespace Mortevielle {

extern Common::String delig;

//static void cinq_huit(char &c, int &ind, byte &pt, bool &the_end);
extern void deline(int num , char *l , int &tl);
//static int l_motsuiv(int p, char *ch, int &tab);
extern void afftex(char *ch, int x, int y, int dx, int dy, int typ);

} // End of namespace Mortevielle
#endif
