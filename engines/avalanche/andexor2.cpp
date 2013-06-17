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

/* Trippancy IV - original file */
#include "graph.h"

namespace Avalanche {

const integer taboo = cyan;

struct adxtype {
           varying_string<12> name; /* name of character */
           byte num; /* number of pictures */
           byte xl,yl; /* x & y lengths of pictures */
           byte seq; /* how many in one stride */
           word size; /* the size of all the pictures */
           byte fgc,bgc; /* foreground & background bubble colours */
};


integer gd,gm;
adxtype adx;
array<0,1,pointer> adxpic;     /* the pictures themselves */
untyped_file f; string x; byte n; integer side2; word bigsize;

void load(string n)
{
    byte z;
 array<1,4,pointer> a;
 untyped_file f; word s;
 varying_string<40> xxx;
 string check;

;
 assign(f,n);
 reset(f,1);
 blockread(f,xxx,41);
 blockread(f,check,13);
 blockread(f,check,31);
 s=imagesize(0,0,getmaxx(),75);
 for( z=1; z <= 2; z ++)
 {;
  getmem(a[z],s);
  blockread(f,a[z],s);
  putimage(0,15+(z-1)*75,a[z],0);
  freemem(a[z],s);
 }
 close(f);
}

void silhouette()
{
    byte x,y,c;
;
 setvisualpage(1); setactivepage(1); setfillstyle(1,15);
 {;
  for( gm=0; gm <= 3; gm ++)
   for( gd=1; gd <= 6; gd ++)
   {;     /* 26,15 */
    side2=adx.xl*6;
    for( y=1; y <= adx.yl; y ++)
     for( x=1; x <= adx.xl; x ++)
     {;
      setactivepage(0);
      c=getpixel((gm / 2)*320+gd*40+x,20+(gm % 2)*40+y);
      setactivepage(1);
      if (c==taboo) 
       putpixel((gm / 2)*side2+gd*adx.xl+x,20+(gm % 2)*adx.yl+y,15);
     }
   }
  bigsize=imagesize(adx.xl+1,21,adx.xl*13,20+adx.yl*2);
  getmem(adxpic[0],bigsize);
  getimage(adx.xl+1,21,adx.xl*13,20+adx.yl*2,adxpic[0]);
  putimage(adx.xl+1,21,adxpic[0],notput);
 }
}

void standard()
{
    byte x,y,c;
;
 setvisualpage(2); setactivepage(2);
 {;
  for( gm=0; gm <= 3; gm ++)
   for( gd=1; gd <= 6; gd ++)
   {;     /* 26,15 */
    for( y=1; y <= adx.yl; y ++)
     for( x=1; x <= adx.xl; x ++)
     {;
      setactivepage(0);
      c=getpixel((gm / 2)*320+gd*40+x,20+(gm % 2)*40+y);
      setactivepage(2);
      if (c!=taboo) 
       putpixel((gm / 2)*side2+gd*adx.xl+x,20+(gm % 2)*adx.yl+y,c);
     }
/*    getmem(adxpic[gm*6+gd,1],adx.size);
    getimage((gm div 2)*side2+gd*xl+x,20+(gm mod 2)*yl+y,
       (gm div 2)*side2+gd*xl*2+x,20+(gm mod 2)*yl*2+y,
       adxpic[gm*6+gd,1]^); */
  }
  getmem(adxpic[1],bigsize);
  getimage(adx.xl+1,21,adx.xl*13,20+adx.yl*2,adxpic[1]);
  putimage(adx.xl+1,21,adxpic[1],notput);
 }
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,""); fillchar(adxpic,sizeof(adxpic),'\261');
 load("v:avalots.avd");
 {;
  adx.name="Avalot";
  adx.num=24; adx.seq=6;
  adx.xl=33; adx.yl=35; /* 35,40 */
  adx.fgc=yellow; adx.bgc=red;

  adx.size=imagesize(40,20,40+adx.xl,60+adx.yl);
 }
 silhouette();
 standard();
 x=string("Sprite file for Avvy - Trippancy IV. Subject to copyright.")+'\32';
 assign(f,"v:sprite1.avd");
 rewrite(f,1);
 blockwrite(f,x[1],59);
 blockwrite(f,adx,sizeof(adx));
 blockwrite(f,bigsize,2);
 for( gm=0; gm <= 1; gm ++)
 {;
  putimage(0,0,adxpic[gm],0);
  blockwrite(f,adxpic[gm],bigsize);  /* next image */
 }
 close(f);
 closegraph();
return EXIT_SUCCESS;
}

} // End of namespace Avalanche.