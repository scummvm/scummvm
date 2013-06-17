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

integer gd,gm;
int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"");
 setactivepage(1); setcolor(9);
 settextstyle(0,0,2);
 outtextxy(0,  0,"This is a test to see whether you can");
 outtextxy(0, 20,"scroll text over the top of a picture.");
 outtextxy(0, 40,"I was wondering whether it would work.");
 outtextxy(0, 60,"Well, does it, or not?");
 outtextxy(0, 80,"If so, it could be useful for the");
 outtextxy(0,100,"Avalot title sequence.");
 outtextxy(0,120,"Good, isn't it?");
 outtextxy(0,140,"The quick brown fox jumps over the");
 outtextxy(0,160,"lazy dog...");
 outtextxy(0,180,"This is the last line...");
 setactivepage(0);
 gm=0;
 for( gd=0; gd <= 640; gd ++)
 {;
  gm += 1; if (gm==8)  gm=1; setcolor(gm);
  line(gd,0,gm,350);
  line(640-gd,0,640-gm,350);
 }
/* for gd:=1 to 15 do
 begin;
  setfillstyle(1,gd); bar(gd*20,10,gd*20+19,100);
 end;*/
/* setfillstyle(1,6); bar(100,100,500,100);*/ gd=getpixel(0,0);
 for( gm=8; gm <= 15; gm ++) setpalette(gm,egayellow);
 for( gd=0; gd <= 199; gd ++)
 {;
  port[0x3c4]=2; port[0x3cf]=4; port[0x3c5]=8; port[0x3cf]=0;
  move(mem[0xa000*80],mem[0xa000*0],16000);
  port[0x3c4]=2; port[0x3cf]=4; port[0x3c5]=8; port[0x3cf]=0;
  move(mem[0xa000*gd*80+16384],mem[0xa000*15920],80);
 }
/* for gd:=0 to 199 do
 begin;
  port[$3C4]:=2; port[$3CF]:=4; port[$3C5]:=8; port[$3CF]:=0;
  move(mem[$A000*160],mem[$A000*0],15920);
  port[$3C4]:=2; port[$3CF]:=4; port[$3C5]:=8; port[$3CF]:=0;
  move(mem[$A000*gd*80+16384],mem[$A000*15920],80);
  move(mem[$A000*15920],mem[$A000*15840],80);
 end;*/
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.