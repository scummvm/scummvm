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
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/*#include "Crt.h"*/

namespace Avalanche {

const char fn[] = "text3.scr";

typedef array<1,3840,char> atype;

file<atype> f;
word fv,ff,fq,st;
char r;
byte tl,bl;
atype q;
atype a /*absolute $B800:0*/;

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 textattr=0; clrscr;
 assign(f,fn); reset(f); f >> q; close(f);
 for( fv=1; fv <= 40; fv ++)
 {;
  if (fv>36)  {; tl=1; bl=24; }
   else {; tl=12-fv / 3; bl=12+fv / 3; }
  for( fq=tl; fq <= bl; fq ++)
   for( ff=80-fv*2; ff <= 80+fv*2; ff ++)
    a[fq*160-ff]=q[fq*160-ff];
  delay(5);
 }
 gotoxy(1,25); textattr=31; clreol; gotoxy(1,24);
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.