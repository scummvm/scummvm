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

/*
  ÛßÜ ÛßÜ ÜßßÜ  ßÛß Ûßß  Üß ßÛß      ÜßÛßÜ  ßÛß ÛÜ  Û ÜÛßß  ßÛß ÛßÜ Ûßß Û
  Ûß  ÛÛ  Û  Û Ü Û  Ûßß ßÜ   Û      Û  Û  Û  Û  Û ÛÜÛ  ßßßÜ  Û  ÛÛ  Ûßß Û
  ß   ß ß  ßß   ßß  ßßß   ß  ß      ß  ß  ß ßßß ß  ßß  ßßß   ß  ß ß ßßß ßßß

                 TRIP             The trippancy handler. (Trippancy 2) */

#define __trip_implementation__


#include "trip.h"


#include "graph.h"
#include "gyro.h"
/*#include "Dos.h"*/

namespace Avalanche {

/*                                                                              */
/*       EGA Graphic Primitive for Turbo Pascal 3.01A, Version 01FEB86.         */
/*       (C) 1986 by Kent Cedola, 2015 Meadow Lake Ct., Norfolk, VA, 23518      */
/*                                                                              */
/*       Description: Write a array of colors in a vertical line.  The current  */
/*       merge setting is used to control the combining of bits.                */
/*                                                                              */
 void gpwtcol(void* buf, integer n)      /* Cedola */
 {;     /*
   inline
     ($1E/$A1/GDCUR_Y/$D1/$E0/$D1/$E0/$03/$06/GDCUR_Y/$05/$A000/$8E/$C0/$8B/$3E/
      GDCUR_X/$8B/$CF/$D1/$EF/$D1/$EF/$D1/$EF/$BA/$03CE/$8A/$26/GDMERGE/$B0/$03/
      $EF/$B8/$0205/$EF/$B0/$08/$EE/$42/$B0/$80/$80/$E1/$07/$D2/$C8/$EE/$8B/$4E/
      $04/$C5/$76/$06/$8A/$24/$46/$26/$8A/$1D/$26/$88/$25/$83/$C7/$50/$E2/$F2/
      $B0/$FF/$EE/$4A/$B8/>$05/$EF/$B8/>$03/$EF/$1F); */
 }


void loadtrip()
{
    untyped_file inf;
;
 assign(inf,"t:avvy.trp"); reset(inf,1);
 seek(inf,0x27);
 blockread(inf,pozzes,sizeof(pozzes));
 blockread(inf,blue3,sizeof(blue3)); close(inf);
}

void plot(word count, integer ox,integer oy) /* orig x & y. Page is always 1. */
{
    byte x,y,len;
;
 do {
  len=blue3[count]; if (len==177)  return;
  x=blue3[count+1]; y=blue3[count+2]; count += 3;
  {;
   gdcur_x=x+ox; gdcur_y=y+oy;
   /* fiddle xy coords to match page 1 */
    gdcur_y += 205;   /* 203 */ gdcur_x -= 128; /* 114 */
    if (gdcur_x<0) 
     {; gdcur_x += 640; gdcur_y -= 1; }
   gpwtcol(&blue3[count],len); count += len;
  }
 } while (!false);
}

void boundscheck()
{;
 if (dna.uy>123)  dna.uy=123;
 if (dna.uy<10)  dna.uy=10;
 if (dna.ux<5)  dna.ux=5;
 if (dna.ux>600)  dna.ux=600;
}

void budge()
{;
 if (set::of(up,ul,ur, eos).has(dna.rw))  dna.uy -= 3;
 if (set::of(down,dl,dr, eos).has(dna.rw))  dna.uy += 3;
 if (set::of(left,ul,dl, eos).has(dna.rw))  dna.ux -= 5;
 if (set::of(right,ur,dr, eos).has(dna.rw))  dna.ux += 5;

 boundscheck();

 if (dna.rw>0) 
 {;
  anim += 1; if (anim==7)  anim=1;
 }
}

void tripkey(char dir)
{;
 if (cw!=177)  return;
 {
 dnatype& with = dna; 
 ;
  switch (dir) {
   case 'H': if (with.rw!=up)     {; with.rw=up;    ww=up;    } else with.rw=0; break;
   case 'P': if (with.rw!=down)   {; with.rw=down;  ww=down;  } else with.rw=0; break;
   case 'K': if (with.rw!=left)   {; with.rw=left;  ww=left;  } else with.rw=0; break;
   case 'M': if (with.rw!=right)  {; with.rw=right; ww=right; } else with.rw=0; break;
   case 'I': if (with.rw!=ur)     {; with.rw=ur;    ww=right; } else with.rw=0; break;
   case 'Q': if (with.rw!=dr)     {; with.rw=dr;    ww=right; } else with.rw=0; break;
   case 'O': if (with.rw!=dl)     {; with.rw=dl;    ww=left;  } else with.rw=0; break;
   case 'G': if (with.rw!=ul)     {; with.rw=ul;    ww=left;  } else with.rw=0; break;
  }
  if (with.rw==0) 
  {;
   ux=ppos[0][0]; uy=ppos[0][1]; anim -= 1;
   if (anim==0)  anim=6;
  }
 }
}

void trippancy()
{;
 if ((dna.rw==0) || (cw!=177) || (! dropsok))  return;
 r.ax=11; intr(0x33,r);
 setactivepage(1); off();
 if (ppos[0][1]!=-177) 
 {;
  putimage(ppos[0][0],ppos[0][1],replace[0],0);
 }

 getimage(dna.ux,dna.uy,dna.ux+xw,dna.uy+yw,replace[0]);
 ppos[0][0]=dna.ux; ppos[0][1]=dna.uy;

 plot(pozzes[anim*4+dna.ww-4],dna.ux,dna.uy);

 { void& with = r;  if ((cx==0) && (dx==0))  on();}
 getimage(dna.ux-margin,dna.uy-margin,dna.ux+xw+margin,dna.uy+yw+margin,copier);
 setactivepage(0); off(); putimage(dna.ux-margin,dna.uy-margin,copier,0); on();

 {
                   void& with = mouths[0];  ; x=dna.ux+20; y=dna.uy; }

 budge();
}

} // End of namespace Avalanche.