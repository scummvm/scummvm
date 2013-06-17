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

integer gd,gm;
pointer p; word s;

void mblit(integer x1,integer y1,integer x2,integer y2) /* Minstrel Blitter */
{
    integer yy,len,pp; byte bit; const integer offset = 16384;
;
 x1=x1 / 8; len=((x2 / 8)-x1)+1;
 for( yy=y1; yy <= y2; yy ++)
 {;
  pp=yy*80+x1;
  for( bit=0; bit <= 3; bit ++)
  {;
   port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
   move(mem[0xa000*offset+pp],mem[0xa000*pp],len);
  }
 }
}

const integer fx1 = 100; const integer fy1 = 100; const integer fx2 = 135; const integer fy2 = 145;

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"");
 setactivepage(1); setfillstyle(7,9); bar(0,0,640,200);
 mblit(fx1,fy1,fx2,fy2);
 s=imagesize(fx1,fy1,fx2,fy2); getmem(p,s);
 getimage(fx1,fy1,fx2,fy2,p); setactivepage(0);
 putimage(fx1+100,fy1,p,0); freemem(p,s);
return EXIT_SUCCESS;
}
