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


const string header = string("Datafile for Avalot, copyright (c) 1992.")+'\32';

integer gd,gm,x,y;
untyped_file f;
byte bit;

void plot(char ch, byte x)
{;
 setcolor(blue);     outtextxy(x*80+4,0,ch); outtextxy(x*80+10,0,ch);
 setcolor(lightblue); outtextxy(x*80+5,0,ch); outtextxy(x*80+9,0,ch);
 setcolor(darkgray); outtextxy(x*80+6,0,ch); outtextxy(x*80+8,0,ch);
 setcolor(yellow);     outtextxy(x*80+7,0,ch);
}

void load()
{
    byte z;
 array<1,4,pointer> a;
 varying_string<12> check;
 untyped_file f; word s;
;
 assign(f,"c:\\avalot\\nimstone.avd"); reset(f,1);
 seek(f,85); z=3;
 s=imagesize(0,0,getmaxx(),75);
 getmem(a[z],s);
 blockread(f,a[z],s);
 putimage(0,7,a[z],0);
 freemem(a[z],s); close(f);
}

void spludge(integer x,integer y, string z)
{
    shortint dx,dy;
;
 setcolor(15);
 for( dx=-1; dx <= 1; dx ++)
  for( dy=-1; dy <= 1; dy ++)
   outtextxy(x+dx,y+dy,z);
 setcolor(0);
 outtextxy(x,y,z);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"");
 load();
 settextstyle(4,0,0); setusercharsize(2,1,1,1);
 plot('A',1);
 plot('B',2);
 plot('C',3);
/* rectangle(gd*80,7,56+gd*80,29); */

 setfillstyle(1,1); setcolor(9);
 fillellipse( 97,104,6,4); fillellipse(321,104,6,4);
 fillellipse( 97,131,6,4); fillellipse(321,131,6,4);
 bar(97,100,321,134);
 bar(92,104,326,131);
 setfillstyle(1,9);
 bar(91,103, 91,131); bar(327,104,327,131);
 bar(98, 99,321, 99); bar( 97,135,321,135);

 settextstyle(2,0,0); setusercharsize(20,10,11,10);
 spludge( 99,100,"The Ancient Game of");
 settextstyle(1,0,0); setusercharsize(40,10,10,10);
 spludge( 99,105,"NIM");

 /* now save it all! */

 assign(f,"c:\\avalot\\nim.avd");
 rewrite(f,1);
 blockwrite(f,header[1],length(header));
 for( gd=0; gd <= 3; gd ++)
  for( gm=7; gm <= 29; gm ++)
   for( bit=0; bit <= 3; bit ++)
   {;
    port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
    blockwrite(f,mem[0xa000*gd*10+gm*80],7);
   }
 for( gm=99; gm <= 135; gm ++)
  for( bit=0; bit <= 3; bit ++)
  {;
   port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
   blockwrite(f,mem[0xa000*11+gm*80],30);
  }
 close(f);
return EXIT_SUCCESS;
}
