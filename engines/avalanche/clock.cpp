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

/*#include "Dos.h"*/
#include "graph.h"
/*#include "Crt.h"*/

const integer xm = 511; const integer ym = 184;
integer gd,gm;
word oh,om,/*os,*/h,m,s,s1; char r;

void hand(word ang,word length, byte colour)
{
    arccoordstype a;
;
 if (ang>900)  return;
 setcolor(colour);
 arc(xm,ym,449-ang,450-ang,length);
 getarccoords(a);
           line(xm,ym,a.xend,a.yend); /* "With a do-line???!", Liz said */
}

void chime()
{
    integer gd,gm,fv;
;
 if (oh>177)  return; /* too high- must be first time around */
 fv=h / 30; if (fv==0)  fv=12;
 for( gd=1; gd <= fv; gd ++)
 {;
  for( gm=1; gm <= 3; gm ++)
  {;
   sound(140-gm*30); delay(50-gm*3);
  }
  nosound; if ((cardinal)gd!=oh)  delay(100);
 }
}

void plothands()
{;
 hand(oh,17,brown);
 hand(h,17,yellow);
 hand(om*6,20,brown);
 hand(m*6,20,yellow);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"");
 setfillstyle(1,6); bar(0,0,640,200); oh=17717; om=17717;
 do {
  gettime(h,m,s,s1);
  h=(h % 12)*30+m / 2;
  if (oh!=h)  {; plothands(); chime(); }
  if (om!=m)  plothands();
  oh=h; om=m;
 } while (!false);
return EXIT_SUCCESS;
}