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

#include "graph.h"

namespace Avalanche {

const integer ttp = 81920;
const integer borland = xorput;
const integer mb1 = 2; /* 2 */
const integer mb2 = 4; /* 4 */

integer gd,gm;
pointer p;
word s;

void mblit()
{
    byte bit; longint st;
;
 st=ttp;
 for( bit=0; bit <= 3; bit ++)
 {;
  port[0x3c4]=mb1;
  portw[0x3ce]=copyput*256+3;
  portw[0x3ce]=0x205;
  port[0x3ce]=0x8;
  port[0x3c5]=1 << bit;
  port[0x3cf]=bit;
  move(mem[longint(0xa000)*st],mem[0xa000*0],7200);
 }
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"");
 settextstyle(1,0,7);
 for( gd=0; gd <= 1; gd ++)
 {;
  setactivepage(gd); setcolor(6*gd+6);
  outtextxy(0,0,chr(65+gd));
 }
 s=imagesize(0,0,90,90); setactivepage(0); getmem(p,s);
 getimage(0,0,90,90,p); putimage(100,100,p,0);
 setactivepage(1); getimage(0,0,90,90,p); setactivepage(0);
 putimage(100,100,p,borland);
 mblit();
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.